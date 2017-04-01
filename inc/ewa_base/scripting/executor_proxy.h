
#ifndef __H_EW_SCRIPTING_EXECUTOR_PROXY__
#define __H_EW_SCRIPTING_EXECUTOR_PROXY__


#include "ewa_base/scripting/executor.h"
#include "ewa_base/logging/logtarget.h"
#include "ewa_base/basic/stream_buffer.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE RpcHandler : public CallableObject
{
public:

	virtual int __getindex(Executor& ewsl,const String& cmd);

	Variant Handle(const String& cmd,Variant req);

	Variant Handle(const String& cmd,const String& sreq)
	{
		Variant req(sreq);
		return Handle(cmd,req);
	}

	Variant Handle(const String& cmd)
	{
		Variant req;
		return Handle(cmd,req);
	}

	virtual Variant DoHandle(const String& cmd,Variant& req) =0;

	virtual Variant berror(const String& msg);
	virtual Variant bmessage(const String& msg);
	virtual Variant bresult(Variant& result);
	virtual Variant bunknown(const String& cmd,Variant&);
	virtual Variant bexception(const String& cmd,std::exception& e);

};


class DLLIMPEXP_EWA_BASE RpcHandlerEx : public RpcHandler
{
public:
	typedef Functor<Variant(Variant&)> functor;

	virtual Variant DoHandle(const String& cmd,Variant& req);
	
	indexer_map<String,functor> aHandlers; 
};

class DLLIMPEXP_EWA_BASE RpcHandlerEwsl : public RpcHandler
{
public:

	RpcHandlerEwsl();

	virtual Variant DoHandle(const String& cmd,Variant& req);
	Executor ewsl;
};


EW_LEAVE
#endif
