#include "ewa_base/scripting/executor_proxy.h"

EW_ENTER


ObjectProxyExecutor::ObjectProxyExecutor()
{

}

DataPtrT<ObjectData> ObjectProxyExecutor::handle_req(const DataPtrT<ObjectData>& req)
{
	const ObjectRequestCommand* q=dynamic_cast<const ObjectRequestCommand*>(req.get());
	if(!q) return NULL;

	return handle_req(q->aRequest);
}

DataPtrT<ObjectData> ObjectProxyExecutor::handle_req(const String& cmd)
{
	arr_1t<String> cmds;
	cmds.push_back(cmd);
	return handle_req(cmds);
}

DataPtrT<ObjectData> ObjectProxyExecutor::handle_req(const arr_1t<String>& cmds)
{
	m_pResponse.reset(new ExecutorState);

	Logger logger;
	logger.reset(m_pResponse.get());

	LoggerSwap logold(logger);
	for(size_t i=0;i<cmds.size();i++)
	{
		ewsl.execute(cmds[i]);
	}

	return m_pResponse;
}

void ExecutorState::Serialize(Serializer& ar)
{
	ar & aRecords;
}

void ExecutorState::Handle(const LogRecord& o)
{
	aRecords.push_back(o);
}

void ObjectRequestCommand::Serialize(Serializer& ar)
{
	ar & nRequest & aRequest;
}



DataPtrT<ObjectData> ObjectProxyHolder::handle_req(const DataPtrT<ObjectData>& req)
{
	if(!m_refData) return NULL;
	return m_refData->handle_req(req);
}


DataPtrT<ObjectData> ObjectProxyHolder::handle_req(const String& cmd)
{
	return handle_req(new ObjectRequestCommand(cmd));
}

DataPtrT<ObjectData> ObjectProxyHolder::handle_req(const arr_1t<String>& cmds)
{
	return handle_req(new ObjectRequestCommand(cmds));
}


ExecutorProxy::ExecutorProxy()
{
	SetData(new ObjectProxyExecutor);
}


IMPLEMENT_OBJECT_INFO(ExecutorState,ObjectInfo);
IMPLEMENT_OBJECT_INFO(ObjectRequestCommand,ObjectInfo);

EW_LEAVE
