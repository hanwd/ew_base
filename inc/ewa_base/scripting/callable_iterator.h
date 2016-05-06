#ifndef __EW_SCRIPTING_VARIANT_ITERAOTR_H__
#define __EW_SCRIPTING_VARIANT_ITERAOTR_H__

#include "ewa_base/scripting/executor.h"

EW_ENTER

class DLLIMPEXP_EWA_BASE CallableDataIterator : public CallableObject
{
public:

	template<typename X>
	static inline void push_value(Executor& ewsl,const X& x)
	{
		ewsl.push(x);
	}
	template<typename X,typename Y>
	static inline void push_value(Executor& ewsl,const std::pair<X,Y>& p)
	{
		ewsl.push(p.second);
	}

};

class DLLIMPEXP_EWA_BASE CallableDataIterator1 : public CallableDataIterator
{
public:

};

class DLLIMPEXP_EWA_BASE CallableDataIterator2 : public CallableDataIterator
{
public:

};


template<typename T>
class DLLIMPEXP_EWA_BASE CallableDataIterator1T : public CallableDataIterator1
{
public:

	DataPtrT<ObjectData> obj;
	T it1,it2;

	CallableDataIterator1T(ObjectData* p,T it1_,T it2_):obj(p),it1(it1_),it2(it2_){}


	int __fun_call(Executor& ewsl,int)
	{
		if(it1==it2)
		{
			ewsl.push(false);
			ewsl.push();
		}
		else
		{
			ewsl.push(true);
			push_value(ewsl,*it1++);
		}
		return 2;

	}
};


template<typename T>
class DLLIMPEXP_EWA_BASE CallableDataIteratorPT : public CallableDataIterator1
{
public:

	DataPtrT<ObjectData> obj;
	T it1,it2;
	int64_t idx;

	CallableDataIteratorPT(ObjectData* p,T it1_,T it2_):obj(p),it1(it1_),it2(it2_),idx(0){}


	int __fun_call(Executor& ewsl,int)
	{
		if(it1==it2)
		{
			ewsl.push(false);
			ewsl.push();
			ewsl.push();
		}
		else
		{
			ewsl.push(true);
			ewsl.push(idx++);
			push_value(ewsl,*it1++);
		}
		return 3;

	}
};

template<typename T>
class DLLIMPEXP_EWA_BASE CallableDataIterator2T : public CallableDataIterator2
{
public:

	DataPtrT<ObjectData> obj;
	T it1,it2;

	CallableDataIterator2T(ObjectData* p,T it1_,T it2_):obj(p),it1(it1_),it2(it2_){}


	template<typename X,typename Y>
	static inline void push_value(Executor& ewsl,const std::pair<X,Y>& p)
	{
		ewsl.push(p.first);
		ewsl.push(p.second);
	}

	int __fun_call(Executor& ewsl,int)
	{
		if(it1==it2)
		{
			ewsl.push(false);
			ewsl.push();
			ewsl.push();
		}
		else
		{
			ewsl.push(true);
			push_value(ewsl,*it1++);
		}
		return 3;
	}
};

EW_LEAVE
#endif
