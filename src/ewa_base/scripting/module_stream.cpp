#include "ewa_base/scripting.h"
#include "ewa_base/basic/stream.h"

EW_ENTER


bool CallableWrapT<Stream>::getline(String& val)
{
	size_t i=0;

	for(;;i++)
	{
		if(i==buffer.size())
		{
			char buf[1024];
			int rc=value.recv(buf,1024);
			if(rc>0)
			{
				buffer.append(buf,rc);
			}
			else if(rc<=0)
			{
				if(buffer.empty())
				{
					return false;
				}

				val=buffer;
				buffer.clear();
				return true;
			}
		}

		if(buffer[i]=='\n') break;
	}

	if(i>0&&buffer[i-1]=='\r')
	{
		val.assign(&buffer[0],&buffer[i-1]);
	}
	else
	{
		val.assign(&buffer[0],&buffer[i]);
	}

	buffer.erase(buffer.begin(),buffer.begin()+i+1);
	return true;
}


class CallableFunctionStreamOpen : public CallableFunction
{
public:
	CallableFunctionStreamOpen():CallableFunction("stream.open"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,1,2);
		String fp=ewsl.ci0.nbx[1].get<String>();
		int fg= pm>1 ? variant_cast<int>(ewsl.ci0.nbx[2]):0;
		ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<Stream>().open(fp,fg);
		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamOpen, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamOpen, ObjectInfo);

class CallableFunctionStreamConnect : public CallableFunction
{
public:
	CallableFunctionStreamConnect():CallableFunction("stream.connect"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,2);
		String fp=ewsl.ci0.nbx[1].get<String>();
		int fg= variant_cast<int>(ewsl.ci0.nbx[2]);
		bool flag=ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<Stream>().connect(fp,fg);
		flag=flag;
		ewsl.ci0.nbx[1]=ewsl.ci1.nbp[StackState1::SBASE_THIS];
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamConnect, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamConnect, ObjectInfo);


class CallableFunctionStreamSave : public CallableFunction
{
public:
	CallableFunctionStreamSave():CallableFunction("stream.save"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,1);
		String fp=ewsl.ci0.nbx[1].get<String>();

		bool flag=ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<Stream>().write_to_file(fp);
		flag=flag;
		ewsl.ci0.nbx[1].reset(flag);

		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamSave, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamSave, ObjectInfo);



class CallableFunctionStreamClose : public CallableFunction
{
public:
	CallableFunctionStreamClose():CallableFunction("stream.close"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,0);
		ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<Stream>().close();
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamClose, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamClose, ObjectInfo);

class CallableFunctionStreamFlush : public CallableFunction
{
public:
	CallableFunctionStreamFlush():CallableFunction("stream.flush"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,0);
		ewsl.ci1.nbp[StackState1::SBASE_THIS].ref<Stream>().get_writer()->flush();
		return 0;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamFlush, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamFlush, ObjectInfo);

class CallableFunctionStreamWrite : public CallableFunction
{
public:
	CallableFunctionStreamWrite():CallableFunction("stream.write"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,1);
		String fp=ewsl.ci0.nbx[1].get<String>();
		ewsl.ci0.nbx[1].reset(ewsl.ci1.nbp[StackState1::SBASE_THIS].get<Stream>().send_all(fp.c_str(),fp.size()));
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamWrite, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamWrite, ObjectInfo);


class CallableFunctionStreamGetline : public CallableFunction
{
public:
	CallableFunctionStreamGetline():CallableFunction("stream.getline"){}
	virtual int __fun_call(Executor& ewsl,int pm)
	{
		ewsl.check_pmc(this, pm,0);
		CallableWrapT<Stream>* p=dynamic_cast<CallableWrapT<Stream>*>(ewsl.ci1.nbp[StackState1::SBASE_THIS].kptr());
		if(!p) ewsl.kerror("invalid stream");
		String val;
		if(p->getline(val))
		{
			ewsl.ci0.nbx[1].reset(val);
		}
		else
		{
			ewsl.ci0.nbx[1].clear();
		}
		return 1;
	}
	DECLARE_OBJECT_CACHED_INFO(CallableFunctionStreamGetline, ObjectInfo);
};
IMPLEMENT_OBJECT_INFO(CallableFunctionStreamGetline, ObjectInfo);


template<>
CallableMetatableT<Stream>::CallableMetatableT():CallableMetatable("stream")
{
	value["open"].kptr(new CallableFunctionStreamOpen());
	value["connect"].kptr(new CallableFunctionStreamConnect());
	value["close"].kptr(new CallableFunctionStreamClose());
	value["getline"].kptr(new CallableFunctionStreamGetline());
	value["write"].kptr(new CallableFunctionStreamWrite());
	value["save"].kptr(new CallableFunctionStreamSave());
	value["flush"].kptr(new CallableFunctionStreamFlush());

}

template<>
int CallableMetatableT<Stream>::__fun_call(Executor& ewsl, int pm)
{
	ewsl.ci0.nbx[1].reset<Stream>();
	return 1;
}



CallableMetatable* CallableWrapT<Stream>::GetMetaTable()
{
	return CallableMetatableT<Stream>::sm_info.GetCachedInstance();
}

IMPLEMENT_OBJECT_INFO(CallableWrapT<Stream>,ObjectInfo);
//IMPLEMENT_OBJECT_INFO(CallableMetatableT<Stream>,ObjectInfo);

EW_LEAVE


