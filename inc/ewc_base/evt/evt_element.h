#ifndef __H_EW_UI_VALUEPROXY_ELEMENT__
#define __H_EW_UI_VALUEPROXY_ELEMENT__

#include "ewc_base/evt/evt_command.h"


EW_ENTER


class DLLIMPEXP_EWC_BASE EvtProxyBase : public EvtCommand
{
public:

	typedef EvtCommand basetype;


	DataPtrT<IValueOptionData> opt_data;
	virtual IValueOptionData* GetComboArray(){return opt_data.get();}

	virtual bool InternalTransfer2Model(){return true;}
	virtual bool InternalTransfer2Window(){return true;}

	virtual bool DoWndExecute(IWndParam& cmd);

};

template<typename T>
class EvtProxyT : public EvtProxyBase
{
public:
	T tmp_value;
	virtual Validator* CreateValidator(wxWindow*);


};


template<typename T>
class DLLIMPEXP_EWC_BASE EvtProxyVariantT : public EvtProxyT<T>
{
public:
	typedef VariantTable map_type;
	map_type& values;
	String key;

	EvtProxyVariantT(map_type& m,const String& k):values(m),key(k){}

	virtual bool InternalTransfer2Model()
	{
		values[key].reset(tmp_value);
		return true;
	}

	virtual bool InternalTransfer2Window()
	{
		tmp_value=pl_cast<T>::g(values[key],T());
		return true;
	}
};

template<>
class DLLIMPEXP_EWC_BASE EvtProxyVariantT<int64_t> : public EvtProxyT<int32_t>
{
public:
	typedef VariantTable map_type;
	map_type& values;
	String key;

	EvtProxyVariantT(map_type& m,const String& k):values(m),key(k){}

	virtual bool InternalTransfer2Model()
	{
		values[key].reset(tmp_value);
		return true;
	}

	virtual bool InternalTransfer2Window()
	{
		tmp_value=pl_cast<int64_t>::g(values[key],0);
		return true;
	}
};


template<>
class DLLIMPEXP_EWC_BASE EvtProxyVariantT<BitFlags> : public EvtProxyT<int32_t>
{
public:
	typedef VariantTable map_type;
	map_type& values;
	String key;
	int64_t mask;

	EvtProxyVariantT(map_type& m,const String& k,int64_t b):values(m),key(k),mask(b){}

	virtual bool InternalTransfer2Model()
	{
		int64_t& val(values[key].ref<int64_t>());
		if(tmp_value!=0)
		{
			val|=mask;
		}
		else
		{
			val=val&~mask;
		}
		return true;
	}

	virtual bool InternalTransfer2Window()
	{
		int64_t& val(values[key].ref<int64_t>());
		tmp_value=(val&mask)?1:0;
		return true;
	}
};






template<typename T,typename E=T>
class DLLIMPEXP_EWC_BASE EvtProxyRefT : public EvtProxyT<T>
{
public:
	E& value;
	EvtProxyRefT(E& v):value(v){}

	virtual bool InternalTransfer2Model(){value=static_cast<E>(tmp_value);return true;}
	virtual bool InternalTransfer2Window(){tmp_value=static_cast<T>(value);return true;}

};

template<>
class DLLIMPEXP_EWC_BASE EvtProxyRefT<int32_t,bool> : public EvtProxyT<int32_t>
{
public:
	bool& value;
	EvtProxyRefT(bool& v):value(v){}

	virtual bool InternalTransfer2Model(){value=tmp_value!=0;return true;}
	virtual bool InternalTransfer2Window(){tmp_value=value?1:0;return true;}

};


template<typename T>
class DLLIMPEXP_EWC_BASE EvtProxyRefT<T,T> : public EvtProxyT<T>
{
public:
	T& value;
	EvtProxyRefT(T& v):value(v){}

	virtual bool InternalTransfer2Model(){value=tmp_value;return true;}
	virtual bool InternalTransfer2Window(){tmp_value=value;return true;}
};

class DLLIMPEXP_EWC_BASE EvtProxyFlags : public EvtProxyT<int32_t>
{
public:
	BitFlags& value;
	int nmask;

	EvtProxyFlags(BitFlags& flags,int n):value(flags),nmask(n){}

	virtual bool InternalTransfer2Model(){value.set(nmask,tmp_value!=0);return true;}
	virtual bool InternalTransfer2Window(){tmp_value=value.get(nmask)?1:0;return true;}

};


EW_LEAVE
#endif