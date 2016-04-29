#ifndef __H_EW_BASIC_PLATFORM__
#define __H_EW_BASIC_PLATFORM__

#include "ewa_base/config.h"
#include "ewa_base/basic/atomic.h"

#ifdef EW_WINDOWS
#include "windows.h"
#endif

#include <algorithm>

EW_ENTER


#ifdef EW_WINDOWS
class KO_Policy_handle
{
public:
	typedef void* type;
	typedef type const_reference;
	static type invalid_value()
	{
		return NULL;
	}
	static void destroy(type& o);
};
#else
class KO_Policy_handle
{
public:
	typedef int type;
	typedef type const_reference;
	static type invalid_value()
	{
		return -1;
	}
	static void destroy(type& o);
};
#endif

template<typename P>
class KO_Base
{
protected:
	typedef typename P::type type;
	typedef typename P::const_reference const_reference;

	KO_Base() :m_pHandle(P::invalid_value()), m_pCounter(NULL){}
	KO_Base(const KO_Base& o) :m_pHandle(o.m_pHandle), m_pCounter(o.m_pCounter){}

protected:
	type m_pHandle;
	mutable RefCounter* m_pCounter;
};


template<typename P>
class KO_Handle : public KO_Base<P>
{
public:
	typedef KO_Base<P> basetype;
	typedef typename P::type type;
	typedef typename P::const_reference const_reference;

	KO_Handle(){}

	KO_Handle(const KO_Handle& o) :basetype(o)
	{
		if(m_pCounter)
		{
			m_pCounter->IncUseCount();
		}
	}

	KO_Handle& operator=(const KO_Handle& o)
	{
		if(m_pCounter==o.m_pCounter)
		{
			return *this;
		}

		if (m_pCounter && m_pCounter->DecUseCount())
		{
			m_pCounter = NULL;
			P::destroy(m_pHandle);
		}

		m_pHandle=o.m_pHandle;
		m_pCounter=o.m_pCounter;

		if(m_pCounter)
		{
			m_pCounter->IncUseCount();
		}

		return *this;
	}

	void reset(const_reference v)
	{
		if (m_pCounter && m_pCounter->DecUseCount())
		{
			m_pCounter = NULL;
			P::destroy(m_pHandle);
		}

		m_pHandle=v;

		if(v!=P::invalid_value())
		{
			m_pCounter=new RefCounter();
			m_pCounter->IncUseCount();
		}

	}

	bool close()
	{
		if(!m_pCounter)
		{
			return false;
		}

		if(m_pCounter->DecUseCount())
		{
			P::destroy(m_pHandle);
			m_pCounter=NULL;
			m_pHandle=P::invalid_value();
			return true;
		}
		else
		{
			m_pHandle=P::invalid_value();
			m_pCounter=NULL;
			return false;
		}
	}

	operator const_reference()
	{
		return m_pHandle;
	}

	bool ok()
	{
		return m_pCounter!=NULL;
	}

	~KO_Handle()
	{
		close();
	}

	void swap(KO_Handle& o)
	{
		std::swap(m_pHandle,o.m_pHandle);
		std::swap(m_pCounter,o.m_pCounter);
	}

};

template<typename P>
class KO_Weak : public KO_Base<P>
{
public:

	typedef KO_Base<P> basetype;
	typedef typename P::type type;
	typedef typename P::const_reference const_reference;

	KO_Weak(){}

	KO_Weak(const KO_Base<P>& p) :basetype(p)
	{
		if (m_pCounter)
		{
			m_pCounter->IncWeakCount();
		}
	}

	KO_Weak& operator=(const KO_Base<P>& p)
	{
		if (m_pCounter == p.m_pCounter) return *this;

		if (m_pCounter)
		{
			m_pCounter->DecWeakCount();
		}

		m_pHandle = o.m_pHandle;
		m_pCounter=o.m_pCounter;

		if (m_pCounter)
		{
			m_pCounter->IncWeakCount();
		}

		return *this;
	}


	KO_Handle<P> lock()
	{
		if (!m_pCounter->Lock())
		{
			return KO_Handle<P>();
		}

		KO_Handle<P> handle;
		handle.m_pHandle = m_pHandle;
		handle.m_pCounter=m_pCounter;

		return handle;
	}

	void swap(KO_Handle& o)
	{
		std::swap(m_pHandle, o.m_pHandle);
		std::swap(m_pCounter, o.m_pCounter);
	}
};

class FileAccess
{
public:
	enum
	{
		FLAG_RD=1<<0,
		FLAG_WR=1<<1,
		FLAG_RW=FLAG_RD|FLAG_WR,
		FLAG_CR=1<<3,
		FLAG_APPEND=1<<4,
		FLAG_EXEC=1<<5,
		FLAG_RWX=FLAG_RD|FLAG_WR|FLAG_EXEC,
		FLAG_WC=FLAG_WR|FLAG_CR,
		FLAG_WA=FLAG_WR|FLAG_APPEND,
	};

	static int makeflag(int flag,int fr,int fw,int ex=0)
	{
		int acc=0;
		if(flag&FLAG_RD)
		{
			acc|=fr;
		}
		if(flag&FLAG_WR)
		{
			acc|=fw;
		}
		if(flag&FLAG_EXEC)
		{
			acc|=ex;
		}
		return acc;
	}

#ifdef EW_WINDOWS
	union LargeInteger
	{
		DWORD64 dval;
		DWORD d[2];
	};
#endif

};

EW_LEAVE
#endif
