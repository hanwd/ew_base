
#ifndef __H_EW_SCRIPTING_EXECUTOR_PROXY__
#define __H_EW_SCRIPTING_EXECUTOR_PROXY__


#include "ewa_base/scripting/executor.h"
#include "ewa_base/logging/logtarget.h"
#include "ewa_base/serialization/serializer_buffer.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE ObjectRequestCommand : public ObjectData
{
public:

	ObjectRequestCommand(intptr_t n=0):nRequest(n){}

	ObjectRequestCommand(const arr_1t<String>& s,intptr_t n=0):nRequest(n),aRequest(s)
	{

	}

	ObjectRequestCommand(const String& s,intptr_t n=0):nRequest(n)
	{
		aRequest.push_back(s);
	}

	intptr_t nRequest;
	arr_1t<String> aRequest;

	void Serialize(Serializer& ar);

	DECLARE_OBJECT_INFO(ObjectRequestCommand,ObjectInfo);
};


class DLLIMPEXP_EWA_BASE ObjectProxy : public ObjectData
{
public:
	virtual DataPtrT<ObjectData> handle_req(const DataPtrT<ObjectData>& req)=0;
};


class DLLIMPEXP_EWA_BASE ExecutorState : public LogTarget
{
public:
	arr_1t<LogRecord> aRecords;

	void Serialize(Serializer& ar);
	virtual void Handle(const LogRecord& o);

	DECLARE_OBJECT_INFO(ExecutorState,ObjectInfo);
};



class DLLIMPEXP_EWA_BASE ObjectProxyExecutor : public ObjectProxy
{
public:

	ObjectProxyExecutor();

	virtual DataPtrT<ObjectData> handle_req(const DataPtrT<ObjectData>& req);

	DataPtrT<ObjectData> handle_req(const String& cmd);
	DataPtrT<ObjectData> handle_req(const arr_1t<String>& cmd);

	Executor ewsl;

protected:
	DataPtrT<ExecutorState> m_pResponse;
};


class DLLIMPEXP_EWA_BASE ObjectProxyHolder : public ObjectT<ObjectProxy>
{
public:

	DataPtrT<ObjectData> handle_req(const DataPtrT<ObjectData>& req);
	DataPtrT<ObjectData> handle_req(const String& str);
	DataPtrT<ObjectData> handle_req(const arr_1t<String>& str);

private:
	SerializerBuffer ar;

};



class DLLIMPEXP_EWA_BASE ExecutorProxy : public ObjectProxyHolder
{
public:
	typedef ObjectProxyHolder basetype;
	ExecutorProxy();
};


EW_LEAVE
#endif
