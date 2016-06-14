#include "ewa_base/scripting/executor_proxy.h"

EW_ENTER


Variant RpcHandler::Handle(const String& cmd,Variant req)
{
	try
	{
		return DoHandle(cmd,req);
	}
	catch(std::exception& e)
	{
		return bexception(cmd,e);
	}
}

int RpcHandler::__getindex(Executor& ewsl,const String& cmd)
{
	class VariantCmdHandler : public CallableFunction
	{
	public:
		VariantCmdHandler(const String& cmd):m_sCmd(cmd){}
		String m_sCmd;

		int __fun_call(Executor& ewsl,int pm)
		{
			ewsl.check_pmc(this,pm,0,1);
			RpcHandler* p=dynamic_cast<RpcHandler*>(ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr());
			if(!p) Exception::XError("invalid caller");
			Variant req;
			if(pm>0) req=ewsl.ci0.nbx[1];
			ewsl.ci0.nbx[1]=p->Handle(m_sCmd,req);
			return 1;
		};
			
	};
	ewsl.ci1.nsp[0].kptr(new VariantCmdHandler(cmd));
	return STACK_BALANCED;
}

Variant RpcHandler::berror(const String& msg)
{
	Variant res;
	VariantTable& tb(res.ref<VariantTable>());
	tb["status"].reset("error");
	tb["message"].reset(msg);
	return res;

}

Variant RpcHandler::bmessage(const String& msg)
{
	Variant res;
	VariantTable& tb(res.ref<VariantTable>());
	tb["status"].reset("ok");
	tb["message"].reset(msg);
	return res;
}

Variant RpcHandler::bresult(Variant& result)
{
	Variant res;
	VariantTable& tb(res.ref<VariantTable>());
	tb["status"].reset("ok");
	tb["result"]=result;
	return res;
}

Variant RpcHandler::bunknown(const String& cmd,Variant&)
{
	return berror("unknown request:"+cmd);
}

Variant RpcHandler::bexception(const String& cmd,std::exception& e)
{
	return berror(cmd+":exception:"+e.what());
}

Variant RpcHandlerEx::DoHandle(const String& cmd,Variant& req)
{
	int id=aHandlers.find1(cmd);

	if(id<0||!aHandlers.get(id).second)
	{
		return bunknown(cmd,req);
	}
	else
	{
		return aHandlers.get(id).second(req);
	}
}	
	

RpcHandlerEwsl::RpcHandlerEwsl()
{

}

class CallableLogCache : public ObjectT<LogCache,CallableObject>
{
public:
	typedef ObjectT<LogCache,CallableObject> basetype;

	CallableLogCache()
	{
		SetData(new LogCache);
	}

	void Serialize(Serializer& ar)
	{
		ar & m_refData;
	}

	DECLARE_OBJECT_INFO(CallableLogCache,ObjectInfo);
};

IMPLEMENT_OBJECT_INFO(CallableLogCache,ObjectInfo);

Variant RpcHandlerEwsl::DoHandle(const String& cmd,Variant& req)
{

	if(cmd!="exec")
	{
		return bunknown(cmd,req);
	}

	DataPtrT<CallableLogCache> m_pResponse(new CallableLogCache());

	LoggerSwap logold(m_pResponse->GetData());

	if(arr_xt<Variant>* pval=req.ptr<arr_xt<Variant> >())
	{
		for(size_t i=0;i<pval->size();i++)
		{
			Variant& var((*pval)[i]);
			ewsl.execute(var.ref<String>());
		}
	}
	else if(String* pval=req.ptr<String>())
	{
		ewsl.execute(*pval);
	}
	else if(StringBuffer<char>* pval=req.ptr<StringBuffer<char> >())
	{
		ewsl.execute(*pval);
	}
	else
	{
		return berror("invalid param");
	}

	//for(size_t i=0;i<cmds.size();i++)
	//{
	//	ewsl.execute(cmds[i]);
	//}

	Variant var(m_pResponse);
	return bresult(var);
}

//void ExecutorState::Serialize(Serializer& ar)
//{
//	ar & aRecords;
//}
//
//void ExecutorState::Handle(const LogRecord& o)
//{
//	aRecords.push_back(o);
//}

//void ObjectRequestCommand::Serialize(Serializer& ar)
//{
//	ar & nRequest & aRequest;
//}

//
//
//DataPtrT<ObjectData> ObjectProxyHolder::handle_req(const DataPtrT<ObjectData>& req)
//{
//	if(!m_refData) return NULL;
//	return m_refData->handle_req(req);
//}
//
//
//DataPtrT<ObjectData> ObjectProxyHolder::handle_req(const String& cmd)
//{
//	return handle_req(new ObjectRequestCommand(cmd));
//}
//
//DataPtrT<ObjectData> ObjectProxyHolder::handle_req(const arr_1t<String>& cmds)
//{
//	return handle_req(new ObjectRequestCommand(cmds));
//}
//
//
//ExecutorProxy::ExecutorProxy()
//{
//	SetData(new ObjectProxyExecutor);
//}


//IMPLEMENT_OBJECT_INFO(ExecutorState,ObjectInfo);
//IMPLEMENT_OBJECT_INFO(ObjectRequestCommand,ObjectInfo);

EW_LEAVE
