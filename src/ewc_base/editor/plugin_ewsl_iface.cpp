#include "plugin_ewsl_caret.h"
#include "ewc_base/wnd/wnd_manager.h"

EW_ENTER


EwslIface::EwslIface(MvcModel& target):Target(target)
{
	busy=false;
	model=new DataModelTable();

	ewsl.set_time_limit(TimeSpan::MilliSeconds(3000));
	ewsl.set_page_limit(1024 * 1024 * 512);
}

EwslIface::~EwslIface()
{
	model->DecRef();
}

void EwslIface::OnExecuteDone(int param1)
{

	if(!busy) return;
	if(!iface) return;

	arr_1t<DataPtrT<ObjectData> > tmps;
	{
		LockGuard<AtomicSpin> lock(spin);
		tmps.swap(m_aPendingRes);
	}

	if(tmps.empty()) return;

	iface->AppendText("\n");
	for(size_t i=0;i<tmps.size();i++)
	{
		LogCache* q=dynamic_cast<LogCache*>(tmps[i].get());
		if(!q) continue;

		DataPtrT<LogCache> res(q);

		bool last_ln = true;

		for(size_t j=0;j<q->aRecords.size();j++)
		{
			LogRecord& rcd(q->aRecords[j]);
			if(rcd.m_nLevel==LOGLEVEL_COMMAND)
			{
				if(rcd.m_sMessage=="clc")
				{
					iface->ClearAll();
				}
				last_ln = true;
			}
			else if (rcd.m_nLevel == LOGLEVEL_PRINT)
			{
				iface->AppendText(str2wx(rcd.m_sMessage));
				last_ln = false;
			}
			else if (rcd.m_nLevel == LOGLEVEL_PRINTLN)
			{
				if (!last_ln) iface->AppendText("\n");
				iface->AppendText(str2wx(rcd.m_sMessage)+"\n");
				last_ln = true;
			}
			else if (rcd.m_nLevel == LOGLEVEL_ERROR)
			{
				if (!last_ln) iface->AppendText("\n");
				iface->AppendText("ERROR:"+str2wx(rcd.m_sMessage)+"\n");
				last_ln = true;
			}
			else
			{
				if (!last_ln) iface->AppendText("\n");
				iface->AppendText(str2wx(rcd.m_sMessage)+"\n");
				last_ln = true;
			}
		}
	}


	model->Update(ewsl.tb1);
	iface->CmdPrepare(EwslCaret::CARET_NEWLINE);

}

void EwslIface::DoExecuteCmds()
{

	while(1)
	{
		arr_1t<String> tmps;
		{
			LockGuard<AtomicSpin> lock(spin);
			tmps.swap(m_aPendingCmd);
		}
		if(tmps.empty())
		{
			break;
		}

		DataPtrT<LogCache> dat(new LogCache);
		Logger logger;
		logger.reset(dat.get());
		{
			LoggerSwap logold(logger);
			for(size_t i=0;i<tmps.size();i++)
			{
				ewsl.execute(tmps[i]);
			}
		}
		{
			LockGuard<AtomicSpin> lock(spin);
			m_aPendingRes.push_back(dat);
		}
	}

	WndManager::current().evtmgr.pending("Ewsl.Done");
}



void EwslIface::PendingExecute(const String& cmd)
{
	EW_ASSERT(!busy);
	if(cmd=="") return;
	busy=true;
	{
		LockGuard<AtomicSpin> lock(spin);
		m_aPendingCmd.push_back(cmd);
	}

	DoExecuteCmds();
}


void EwslIface::reset(EwslCaret* face)
{
	iface.reset(face);
}


EW_LEAVE
