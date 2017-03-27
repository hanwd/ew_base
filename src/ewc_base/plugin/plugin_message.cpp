#include "ewc_base/plugin/plugin_message.h"
#include "ewc_base/plugin/plugin_manager.h"
#include "ewc_base/evt/evt_manager.h"
#include "ewc_base/wnd/wnd_updator.h"
#include "ewc_base/wnd/wnd_model.h"
#include "ewc_base/wnd/impl_wx/window.h"
#include "ewc_base/app/res_manager.h"

#include <wx/dataview.h>

EW_ENTER

PluginMessage::PluginMessage(WndManager& w):basetype(w,"Plugin.Message")
{
	m_aDependency.push_back("Plugin.Basic");
}


class EvtCommandWindowMessage;

class LogTargetMessage : public LogTarget
{
public:
	LogTargetMessage(EvtCommandWindowMessage& t):Target(t){}
	EvtCommandWindowMessage& Target;
	void Handle(const LogRecord& o);
};


// 消息窗口列定义
class IColumnRecordDesc : public IValueColumnT<LogRecord>
{
public:

	IColumnRecordDesc()
	{
		type=COLUMNTYPE_STRING;
		name="desc";
		width=280;
	}

	virtual bool SetValue(itemtype&,const ICellInfo &)
	{
		return false;
	}

	virtual void GetValue(const itemtype& val,ICellInfo &cell) const
	{
		cell.variant=str2wx(val.m_sMessage);
	}

};
// 消息窗口列定义
class IColumnRecordSrc : public IValueColumnT<LogRecord>
{
public:

	IColumnRecordSrc()
	{
		type=COLUMNTYPE_STRING;
		name="src";
		width=40;
	}

	virtual bool SetValue(itemtype&,const ICellInfo &)
	{
		return false;
	}

	virtual void GetValue(const itemtype& val,ICellInfo &cell) const
	{
		cell.variant=str2wx(LogSource::current()[val.m_nSrc]);
	}

};
// 消息窗口列定义
class IColumnRecordType : public IValueColumnT<LogRecord>
{
public:

	IColumnRecordType()
	{
		type=COLUMNTYPE_ICON;
		name="";
		width=32;

		icon[0]=ResManager::current().icons.get_bitmap("Message",16);
		icon[1]=ResManager::current().icons.get_bitmap("Warning",16);
		icon[2]=ResManager::current().icons.get_bitmap("Error",16);

	}

	virtual bool SetValue(itemtype&,const ICellInfo &)
	{
		return false;
	}

	virtual void GetValue(const itemtype& val,ICellInfo &cell) const
	{
		if(val.m_nLevel==LOGLEVEL_WARNING)
		{
			cell.variant << icon[1];
		}
		else if(val.m_nLevel==LOGLEVEL_ERROR)
		{
			cell.variant << icon[2];
		}
		else
		{
			cell.variant << icon[0];
		}
	}

	wxBitmap icon[3];
};

// 消息窗口列定义
class IColumnRecordTime : public IValueColumnT<LogRecord>
{
public:


	IColumnRecordTime()
	{
		type=COLUMNTYPE_STRING;
		width=80;
		name="time";
	}

	virtual bool SetValue(itemtype&,const ICellInfo &)
	{
		return false;
	}

	virtual void GetValue(const itemtype& val,ICellInfo &cell) const
	{
		cell.variant=str2wx(val.m_tStamp.Format("%H:%M:%S"));
	}

};

// 消息过滤器基类，子类必须在m_aLogLevel中定义关联的LogLevel。
class EvtMsgBase : public EvtCommand
{
public:
	EvtCommandWindowMessage& Target;

	EvtMsgBase(const String& s,EvtCommandWindowMessage& t):EvtCommand(s),Target(t)
	{
		flags.add(FLAG_CHECK|FLAG_CHECKED);
	}

	void DoUpdateCtrl(IUpdParam& upd);

	virtual bool CmdExecute(ICmdParam& cmd);

	arr_1t<int> m_aLogLevel;

	String m_sFormat;

};

class EvtMsgError : public EvtMsgBase
{
public:
	EvtMsgError(EvtCommandWindowMessage& t):EvtMsgBase("Msg.Error",t)
	{
		m_bmpParam=ResManager::current().icons.get("Error");
		m_sFormat="%d Errors";
		m_aLogLevel.append(LOGLEVEL_ERROR);
		m_aLogLevel.append(LOGLEVEL_FATAL);
	}
};

class EvtMsgWarning : public EvtMsgBase
{
public:
	EvtMsgWarning(EvtCommandWindowMessage& t):EvtMsgBase("Msg.Warning",t)
	{
		m_bmpParam=ResManager::current().icons.get("Warning");
		m_sFormat="%d Warnings";
		m_aLogLevel.append(LOGLEVEL_WARNING);
	}
};

class EvtMsgMessage : public EvtMsgBase
{
public:
	EvtMsgMessage(EvtCommandWindowMessage& t):EvtMsgBase("Msg.Message",t)
	{
		m_bmpParam=ResManager::current().icons.get("Message");
		m_sFormat="%d Messages";
		m_aLogLevel.append(LOGLEVEL_MESSAGE);
		m_aLogLevel.append(LOGLEVEL_TRACE);
	}
};

// 消息窗口关联的Model。
class IMessageModel : public ::wxDataViewVirtualListModel
{
public:
	EvtCommandWindowMessage& Target;

	IMessageModel(EvtCommandWindowMessage& t):Target(t)
	{
		m_aColumns.push_back(new IColumnRecordType);
		m_aColumns.push_back(new IColumnRecordSrc);
		m_aColumns.push_back(new IColumnRecordDesc);
		m_aColumns.push_back(new IColumnRecordTime);
	}

    virtual bool GetAttrByRow( unsigned int row, unsigned int col,
                               wxDataViewItemAttr &attr ) const
	{
		wxColour colors[]={*wxWHITE,wxColor(230,230,230)};
		attr.SetBackgroundColour(colors[(row)%(sizeof(colors)/sizeof(colors[0]))]);
		return true;
	}

    virtual void GetValueByRow( wxVariant &variant,
                                unsigned int row, unsigned int col ) const;

    virtual bool SetValueByRow( const wxVariant &,
                                unsigned int, unsigned int)
	{
		return false;
	}

    virtual unsigned int GetColumnCount() const
    {
        return m_aColumns.size();
    }

    virtual wxString GetColumnType( unsigned int col ) const
    {
		if(col>=m_aColumns.size()) return "";

		switch(m_aColumns[col]->type)
		{
		case COLUMNTYPE_ICON:
			return "wxBitmap";
		case COLUMNTYPE_ICONTEXT:
			return "wxDataViewIconText";
		default:
			return "string";
		}
    }


	arr_1t<DataPtrT<IValueColumnT<LogRecord> > > m_aColumns;

};

class IMessageDataView : public wxDataViewCtrl
{
public:
	IMessageDataView(wxWindow* p):wxDataViewCtrl(p,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxDV_MULTIPLE){}

	// 强制指定行可见，用于控制滚动条滚动到最新的消息
	void EnsureRowVisible(int row)
	{
		wxDataViewItem item=GetItemByRow(row);
		if(item.IsOk())
		{
			EnsureVisible(item);
		}
	}
};


class EvtMsgSearch : public EvtCommand
{
public:
	EvtCommandWindowMessage& Target;
	EvtMsgSearch(EvtCommandWindowMessage& t):Target(t),EvtCommand("Msg.Search")
	{
		m_bmpParam=ResManager::current().icons.get("Find");
	}

	bool CmdExecute(ICmdParam& cmd);

};

// 消息窗口类
class IMessageView : public wxPanel
{
public:
	EvtCommandWindowMessage& Target;
	IMessageDataView* m_pDataView;


	IMessageView(EvtCommandWindowMessage& t,wxWindow* p):Target(t),wxPanel(p, wxID_ANY, wxDefaultPosition,wxDefaultSize)
	{

		this->SetMinSize(wxSize(200,48));

		EvtManager& ec(EvtManager::current());
		ec.append(new EvtMsgError(t));
		ec.append(new EvtMsgMessage(t));
		ec.append(new EvtMsgWarning(t));
		ec.append(new EvtMsgSearch(t));

		ec.gp_beg("tb.MsgLevel");
			ec.gp_add("Msg.Error");
			ec.gp_add("Msg.Warning");
			ec.gp_add("Msg.Message");
		ec.gp_end();

		ec.gp_beg("tb.MsgSearch");
			ec.gp_add(new EvtCommandText("Msg.Search.Text"));
			ec.gp_add("Msg.Search");
		ec.gp_end();

		WndMaker km(this);
		km.row();
			km.col(WndProperty().flags(IDefs::IWND_EXPAND));
				km.add(ec["tb.MsgLevel"].cast_group()->CreateTbar(ICtlParam("toolbar",16,this)),WndProperty());
				km.add("space",WndProperty().propotion(1));
				km.add(ec["tb.MsgSearch"].cast_group()->CreateTbar(ICtlParam("toolbar",16,this)),WndProperty());
			km.end();
			km.add(m_pDataView=new IMessageDataView(this),WndProperty().propotion(1).flags(IDefs::IWND_EXPAND));
		km.end();

	}

	void AssociateModel(IMessageModel* model)
	{
		for(size_t i=0;i<model->m_aColumns.size();i++)
		{
			m_pDataView->AppendColumn(IValueColumn::CreateDataViewColumn(*model->m_aColumns[i],i));
		}
		m_pDataView->AssociateModel(model);
	}

	void EnsureRowVisible(int row)
	{
		m_pDataView->EnsureRowVisible(row);
	}
};


// 在新消息显示后执行，控制滚动条滚动到最新的消息上。
class EvtCommandMessageAdded : public EvtCommand
{
public:
	EvtCommandWindowMessage& Target;
	EvtCommandMessageAdded(EvtCommandWindowMessage& t):Target(t),EvtCommand("MessageAdded"){}
	bool CmdExecute(ICmdParam& cmd);
};

// 消息窗口相关的EvtCommandWindow类。
class EvtCommandWindowMessage : public EvtCommandWindow
{
public:

	WndManager& wm;

	DataPtrT<EvtListener> m_pLoggerListener;

	wxObjectDataPtr<IMessageModel> m_pListModel;
	IMessageView* m_pListView;

	bool m_aLogLevelEnabled[LOGLEVEL_MAX];
	int  m_nLogLevelCount[LOGLEVEL_MAX];
	String m_sSearchText;

	void DoUpdateCtrl(IUpdParam& upd)
	{
		EvtCommandWindow::DoUpdateCtrl(upd);
		UpdateRecords();
	}

	~EvtCommandWindowMessage()
	{

	}

	bool OnCmdEvent(ICmdParam& cmd,int phase)
	{
		if(cmd.evtptr->m_sId=="CloseFrame" && phase==IDefs::PHASE_POSTCALL)
		{
			Logger::def(new LogNull);
		}
		return true;
	}

	EvtCommandWindowMessage():EvtCommandWindow(_kT("Message")),wm(WndManager::current())
	{
		memset(m_aLogLevelEnabled,-1,sizeof(m_aLogLevelEnabled));
		memset(m_nLogLevelCount,0,sizeof(m_nLogLevelCount));

		m_nMaxSize=4000;
		m_nCapacity=5000;
		m_nRdPos=0;
		m_nWrPos=0;

		m_aRecords.resize(m_nCapacity);

		EvtManager::current().append(new EvtCommandMessageAdded(*this));

        m_pListModel = new IMessageModel(*this);
		m_pListView = new IMessageView(*this,WndModel::current().GetWindow());

		//m_pListView->Connect(wxID_ANY,wxEVT_CHILD_FOCUS,wxChildFocusEventHandler(MyWindow::OnFocus));


		m_pListView->SetName(str2wx(m_sId));
        m_pListView->AssociateModel( m_pListModel.get() );

		m_pListView->SetSize(wxSize(480,240));
		m_pListView->SetMinSize(wxSize(120,120));

		SetWindow(m_pListView);

		LogTarget* ptarget=new LogTargetMessage(*this);
		WndManager::current().logptr.SetData(ptarget);
		Logger::def(ptarget);

		wm.evtmgr["CloseFrame"].AttachListener(this);

	}

	void Handle(const LogRecord& o)
	{
		LockGuard<AtomicMutex> lock(m_tLock);
		m_aRecordsTemp.push_back(o);
		wm.evtmgr.update(m_sId);
	}


	// called by main thread only
	void ClearRecords()
	{
		LockGuard<AtomicMutex> lock(m_tLock);
		m_aRecordsTemp.clear();
		m_nRdPos=m_nWrPos;
		UpdateModel();
	}

	// called by main thread only
	void UpdateModel();

	// called by main thread only
	void UpdateRecords()
	{

		arr_1t<LogRecord> _aTemps;
		m_tLock.lock();
		_aTemps.swap(m_aRecordsTemp);
		m_tLock.unlock();

		if(_aTemps.empty()) return;

		int ns=(m_nWrPos+m_nCapacity-m_nRdPos)%m_nCapacity;
		int nd=ns+(int)_aTemps.size()-m_nMaxSize;
		if(nd>0)
		{
			m_nRdPos+=nd;
		}

		for(size_t i=0;i<_aTemps.size();i++)
		{
			m_aRecords[m_nWrPos++%m_nCapacity]=_aTemps[i];
		}
		
		UpdateModel();

	}


	AtomicMutex m_tLock;
	indexer_map<int,int> m_aSrcCount;

	// 经过过滤后，最终显示的Record的id。
	arr_1t<int> m_aRecordsReal;

	// 所有的Record
	arr_1t<LogRecord> m_aRecords;
	int m_nMaxSize;
	int m_nCapacity;
	int m_nRdPos;
	int m_nWrPos;

	// 待添加的Record
	arr_1t<LogRecord> m_aRecordsTemp;

};

void LogTargetMessage::Handle(const LogRecord& o)
{
	Target.Handle(o);
}

void IMessageModel::GetValueByRow( wxVariant &variant,
                            unsigned int row, unsigned int col ) const
{
	if(row>=Target.m_aRecordsReal.size()||col>=m_aColumns.size()) return;
	ICellInfo cell(variant,row,col);
	m_aColumns[col]->GetValue(Target.m_aRecords[Target.m_aRecordsReal[row]],cell);
}

bool EvtMsgSearch::CmdExecute(ICmdParam& cmd)
{
	cmd.param1=1;

	//EvtManager::current()["Msg.Search.Text"].StdExecute(cmd);
	//if(cmd.extra.get(Target.m_sSearchText))
	//{
	//	Target.UpdateModel();
	//}
	return true;
}

// 设置关联的LogLevel的过滤状态，并刷新。
bool EvtMsgBase::CmdExecute(ICmdParam& cmd)
{
	flags.set(FLAG_CHECKED,cmd.param1!=0);
	for(size_t i=0;i<m_aLogLevel.size();i++)
	{
		int lv=m_aLogLevel[i];
		Target.m_aLogLevelEnabled[lv]=flags.get(FLAG_CHECKED);
	}

	Target.UpdateModel();
	return true;
}


void EvtMsgBase::DoUpdateCtrl(IUpdParam& upd)
{
	int lv_count=0;
	for(size_t i=0;i<m_aLogLevel.size();i++)
	{
		int lv=m_aLogLevel[i];
		lv_count+=Target.m_nLogLevelCount[lv];
	}

	this->m_sLabel=String::Format(m_sFormat,lv_count);

	EvtCommand::DoUpdateCtrl(upd);

}

bool EvtCommandMessageAdded::CmdExecute(ICmdParam& cmd)
{
	Target.m_pListView->EnsureRowVisible(cmd.param1);
	return true;
}

void EvtCommandWindowMessage::UpdateModel()
{
	m_aSrcCount.clear();
	m_aRecordsReal.clear();
	memset(m_nLogLevelCount,0,sizeof(m_nLogLevelCount));

	for(int i=m_nRdPos;i!=m_nWrPos;i=(i+1)%m_nCapacity)
	{
		int lv=m_aRecords[i].m_nLevel;
		if(uint32_t(lv)>LOGLEVEL_MAX) continue;

		m_nLogLevelCount[lv]++;
		if(!m_aLogLevelEnabled[lv])
		{
			continue;
		}

		if(!m_sSearchText.empty())
		{
			const char* c=::strstr(m_aRecords[i].m_sMessage.c_str(),m_sSearchText.c_str());
			if(c==NULL) continue;
		}

		m_aSrcCount[m_aRecords[i].m_nSrc]++;
		m_aRecordsReal.append(i);
	}

	unsigned _nRowCount=m_aRecordsReal.size();
	m_pListModel->Reset(_nRowCount);

	EvtManager::current()["TB_MSG"].UpdateCtrl();

	//延时事件，通知消息窗口滚动动最新的消息
	if(_nRowCount>0) EvtManager::current().pending("MessageAdded",_nRowCount-1);

}


bool PluginMessage::OnAttach()
{

	EvtManager& ec(wm.evtmgr);

	ec.append(new EvtCommandWindowMessage);

	ec.gp_beg("MainWindow");
		ec.gp_add("Message");
	ec.gp_end();

	WndUpdator::current().gp_add("View",1);

	return true;
}

IMPLEMENT_IPLUGIN(PluginMessage)

EW_LEAVE
