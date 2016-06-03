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
class DLLIMPEXP_EWA_BASE KO_Policy_handle
{
public:
	typedef void* type;
	typedef type const_reference;
	static inline type invalid_value()
	{
		return NULL;
	}
	static void destroy(type& o);
	static type duplicate(type v,int h=0);
};
#else
class DLLIMPEXP_EWA_BASE KO_Policy_handle
{
public:
	typedef int type;
	typedef type const_reference;
	static inline type invalid_value()
	{
		return -1;
	}
	static void destroy(type& o);
	static type duplicate(type v);
};
#endif

template<typename T>
class KO_Policy_pointer
{
public:
	typedef T* type;
	typedef type const_reference;
	static type invalid_value(){return NULL;}
	static void destroy(type& o);
};


template<typename P>
class KO_Base
{
public:
	RefCounter* counter(){return m_pCounter;}

protected:
	typedef typename P::type type;
	typedef typename P::const_reference const_reference;

	KO_Base() :m_pHandle(P::invalid_value()), m_pCounter(NULL){}
	KO_Base(const KO_Base& o) :m_pHandle(o.m_pHandle), m_pCounter(o.m_pCounter){}

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
	KO_Handle(const_reference v)
	{
		reset(v);
	}

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

	template<typename P2>
	void reset(const_reference v,KO_Base<P2>& c)
	{
		EW_ASSERT(c.counter()!=NULL);
		EW_ASSERT(v!=P::invalid_value());

		if (m_pCounter && m_pCounter->DecUseCount())
		{
			m_pCounter = NULL;
			P::destroy(m_pHandle);
		}

		m_pHandle=v;		
		m_pCounter=c.counter();
		m_pCounter->IncUseCount();	
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

	type release()
	{
		if(!m_pCounter)
		{
			return P::invalid_value();
		}

		if(m_pCounter->DecUseCount())
		{
			m_pCounter=NULL;
			type p=m_pHandle;
			m_pHandle=P::invalid_value();
			return p;
		}
		else
		{
			m_pHandle=P::invalid_value();
			m_pCounter=NULL;
			return P::invalid_value();
		}
	}

	operator const_reference()
	{
		return m_pHandle;
	}

	const_reference get()
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


enum
{
	SEEKTYPE_BEG,
	SEEKTYPE_CUR,
	SEEKTYPE_END
};

enum
{
	FLAG_READER_FAILBIT=1<<0,
	FLAG_WRITER_FAILBIT=1<<1,

};

enum
{
	FLAG_FILE_RD=1<<0,
	FLAG_FILE_WR=1<<1,
	FLAG_FILE_RW=FLAG_FILE_RD|FLAG_FILE_WR,
	FLAG_FILE_CR=1<<3,
	FLAG_FILE_APPEND=1<<4,
	FLAG_FILE_EXEC=1<<5,
	FLAG_FILE_TRUNCATE=1<<6,

	FLAG_FILE_RWX=FLAG_FILE_RD|FLAG_FILE_WR|FLAG_FILE_EXEC,
	FLAG_FILE_WC=FLAG_FILE_WR|FLAG_FILE_CR,
	FLAG_FILE_WA=FLAG_FILE_WR|FLAG_FILE_APPEND,

	FILE_TYPE_TEXT_BOM		=1<<10,
	FILE_TYPE_BINARY		=0<<11,
	FILE_TYPE_TEXT			=1<<11,
	FILE_TYPE_TEXT_ANSI		=2<<11,
	FILE_TYPE_TEXT_UTF8		=3<<11,
	FILE_TYPE_TEXT_UTF16_BE	=4<<11,
	FILE_TYPE_TEXT_UTF16_LE	=5<<11,
	FILE_TYPE_MASK			=7<<11,

};


class FileAccess
{
public:

	static int makeflag(int flag,int fr,int fw,int ex=0)
	{
		int acc=0;
		if(flag&FLAG_FILE_RD)
		{
			acc|=fr;
		}
		if(flag&FLAG_FILE_WR)
		{
			acc|=fw;
		}
		if(flag&FLAG_FILE_EXEC)
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
