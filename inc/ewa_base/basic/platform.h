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
	static inline type invalid_value(){return NULL;}
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
	static void destroy(type& o){delete o;o=NULL;}
};


template<typename P>
class KO_Base
{
public:

	RefCounter* counter() const {return m_pCounter;}

	int use_count() const
	{
		return m_pCounter? m_pCounter->GetUseCount():0;
	}

protected:
	typedef typename P::type type;
	typedef typename P::const_reference const_reference;

	KO_Base():m_pHandle(P::invalid_value()), m_pCounter(NULL){}
	KO_Base(const KO_Base& o) :m_pHandle(o.m_pHandle), m_pCounter(o.m_pCounter){}

	type m_pHandle;
	mutable RefCounter* m_pCounter;
};

template<typename P>
class KO_Weak;


template<typename P>
class KO_Handle : public KO_Base<P>
{
public:

	friend class KO_Weak<P>;

	typedef KO_Base<P> basetype;
	typedef typename P::type type;
	typedef typename P::const_reference const_reference;

	using basetype::m_pHandle;
	using basetype::m_pCounter;

	KO_Handle(){}
	explicit KO_Handle(const_reference v)
	{
		if(v!=P::invalid_value())
		{
			m_pCounter=new RefCounter(1);
			m_pHandle=v;
		}
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
		reset(o.m_pHandle,o.m_pCounter);
		return *this;
	}


	void reset(const_reference v,RefCounter* p)
	{
		if(m_pCounter==p)
		{
			m_pHandle=v;
			return;
		}

		if (m_pCounter && m_pCounter->DecUseCount())
		{
			P::destroy(m_pHandle);
		}

		m_pHandle=v;
		m_pCounter=p;

		if(m_pCounter)
		{
			m_pCounter->IncUseCount();
		}

	}

	void reset(const_reference v)
	{
		if (m_pCounter && m_pCounter->DecUseCount())
		{
			P::destroy(m_pHandle);
		}

		m_pCounter=NULL;
		m_pHandle=v;

		if(v!=P::invalid_value())
		{
			m_pCounter=new RefCounter(1);
		}

	}

	void reset()
	{
		if(!m_pCounter) return;

		if(m_pCounter->DecUseCount())
		{
			P::destroy(m_pHandle);
			m_pCounter=NULL;
		}
		else
		{
			m_pHandle=P::invalid_value();
			m_pCounter=NULL;
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

	bool unique() const
	{
		return this->use_count()==1;
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
		reset();
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

	using basetype::m_pCounter;
	using basetype::m_pHandle;

	KO_Weak(){}

	KO_Weak(const KO_Base<P>& p) :basetype(p)
	{
		if (m_pCounter)
		{
			m_pCounter->IncWeakCount();
		}
	}
	~KO_Weak()
	{
		reset();
	}

	KO_Weak& operator=(const KO_Base<P>& p)
	{
		if (m_pCounter == p.m_pCounter)
		{
			return *this;
		}

		if (m_pCounter)
		{
			m_pCounter->DecWeakCount();
		}

		basetype::operator=(p);

		if (m_pCounter)
		{
			m_pCounter->IncWeakCount();
		}

		return *this;
	}

	bool expired() const
	{
		return !m_pCounter || m_pCounter->GetUseCount()==0;
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

	void reset()
	{
		if(m_pCounter)
		{
			m_pCounter->DecWeakCount();
			m_pCounter=NULL;
		}
	}

	const_reference unsafe_get()
	{
		return m_pHandle;
	}

	void swap(KO_Weak& o)
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
	FILE_TYPE_BINARY		=0,
	FILE_TYPE_TEXT			=1,
	FILE_TYPE_TEXT_ANSI		=2,
	FILE_TYPE_TEXT_UTF8		=3,
	FILE_TYPE_TEXT_UTF16_BE	=4,
	FILE_TYPE_TEXT_UTF16_LE	=5,
	FILE_TYPE_MASK			=7,
	FILE_TYPE_TEXT_BOM		=8,

	FLAG_FILE_BASE		=1<<4,
	FLAG_FILE_RD		=FLAG_FILE_BASE<<0,
	FLAG_FILE_WR		=FLAG_FILE_BASE<<1,
	FLAG_FILE_CR		=FLAG_FILE_BASE<<3,
	FLAG_FILE_APPEND	=FLAG_FILE_BASE<<4,
	FLAG_FILE_EXEC		=FLAG_FILE_BASE<<5,
	FLAG_FILE_TRUNCATE	=FLAG_FILE_BASE<<6,

	FLAG_FILE_RW		=FLAG_FILE_RD|FLAG_FILE_WR,
	FLAG_FILE_RWX		=FLAG_FILE_RD|FLAG_FILE_WR|FLAG_FILE_EXEC,
	FLAG_FILE_WC		=FLAG_FILE_WR|FLAG_FILE_CR,
	FLAG_FILE_WA		=FLAG_FILE_WR|FLAG_FILE_APPEND,

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
