
#ifndef __H_EW_MEMORY_GC_PTR__
#define __H_EW_MEMORY_GC_PTR__

#include "ewa_base/config.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/collection/arr_1t.h"

EW_ENTER

DLLIMPEXP_EWA_BASE void gc_force_collect();

class DLLIMPEXP_EWA_BASE gc_obj;

template<typename T> class gc_ptr;

template<typename T> gc_ptr<T> gc_new();

enum
{
	GC_TAG_INIT=0,
	GC_TAG_FLAG=1,
	GC_TAG_MARK=2,
};

class DLLIMPEXP_EWA_BASE gc_mark_queue;

class DLLIMPEXP_EWA_BASE gc_obj
{
public:

	friend class gc_state;
	friend class gc_mark_queue;

	inline gc_obj()
	{
		_gc_tags=GC_TAG_INIT;
	}

	inline gc_obj(const gc_obj& o)
	{
		EW_UNUSED(o);
		_gc_tags=GC_TAG_INIT;
	}

	inline gc_obj& operator=(const gc_obj&){return *this;}

	virtual ~gc_obj(){}

protected:

	// mark object that is connected to this
	virtual void _gc_mark(gc_mark_queue& q)
	{
		EW_UNUSED(q);
	}

	gc_obj* _gc_next;;
	int32_t _gc_tags;
};


class DLLIMPEXP_EWA_BASE gc_dlinkptr_type
{
public:
	gc_obj* m_ptr;
	gc_dlinkptr_type* _gc_next;
	gc_dlinkptr_type* _gc_prev;
};

template<typename T>
class gc_ptr : private gc_dlinkptr_type
{
public:

	typedef gc_dlinkptr_type basetype;
	typedef T* pointer;
	typedef const T* const_pointer;

	gc_ptr(T* p=NULL);
	gc_ptr(const gc_ptr& o);

	template<typename G>
	gc_ptr(const gc_ptr<G>& o);

	~gc_ptr();

	inline pointer get()
	{
		return (pointer)m_ptr;
	}

	inline const_pointer get() const
	{
		return (const_pointer)m_ptr;
	}

	void reset(pointer p);

	inline gc_ptr& operator=(const gc_ptr& o)
	{
		reset((pointer)o.get());
		return *this;
	}
	
	template<typename G>
	inline gc_ptr& operator=(const gc_ptr<G>& o)
	{
		reset((G*)o.get());
		return *this;
	}

	inline operator pointer()
	{
		return (pointer)m_ptr;
	}

	inline operator const_pointer() const
	{
		return (const_pointer)m_ptr;
	}		
		
	inline T &operator*()
	{
		return *(pointer)m_ptr;
	}

	inline const T &operator*() const
	{
		return *(const_pointer)m_ptr;
	}

	inline pointer operator->()
	{
		return (pointer)m_ptr;
	}

	inline const T* operator->() const
	{
		return (const_pointer)m_ptr;
	}

	inline operator bool() const
	{
		return m_ptr!=NULL;
	}

	void swap(gc_ptr& o);

};


class DLLIMPEXP_EWA_BASE gc_mark_queue
{
public:
	typedef arr_1t<gc_obj*> impl_type;
	typedef impl_type::iterator iterator;

	inline iterator begin(){return impl.begin();}
	inline iterator end(){return impl.end();}
	inline void swap(gc_mark_queue& o){impl.swap(o.impl);}

	inline bool empty(){return impl.empty();}
	inline void clear(){impl.clear();}
	inline void reset(){impl.resize(0);}

	void append(gc_obj* o);

	template<typename IT>
	void append(IT t1,IT t2);

protected:
	impl_type impl;
};

class DLLIMPEXP_EWA_BASE gc_state
{
public:

	enum
	{
		FLAG_SHOW_GC_INFO	=1<<0,
	};
	
	class addrinfo_type
	{
	public:
		void* addr;
		size_t size;
		inline bool test(void* p)
		{
			return size_t((char*)p-(char*)addr)<size;
		}
	};

	static addrinfo_type tCurrentObject;
	static BitFlags flags;

	static inline void link(gc_obj* p)
	{
		EW_ASSERT(p!=NULL);
		p->_gc_next=pLinkOfAllObject;
		pLinkOfAllObject=p;
		++gc_state::nNumberOfObject;
	}

	inline void insert(gc_dlinkptr_type* p)
	{
		if(!tCurrentObject.test(p))
		{
			tRecursiveMutex.lock();
			
			EW_ASSERT(tPtrLink_head._gc_next!=NULL);
			EW_ASSERT(tPtrLink_head._gc_next->_gc_prev!=NULL);

			p->_gc_next=tPtrLink_head._gc_next;
			p->_gc_prev=&tPtrLink_head;
			tPtrLink_head._gc_next->_gc_prev=p;	
			tPtrLink_head._gc_next=p;

			tRecursiveMutex.unlock();
		}
		else
		{
			p->_gc_next=NULL;
			p->_gc_prev=NULL;
		}
	}

	inline void erase(gc_dlinkptr_type* p)
	{
		if(p->_gc_prev)
		{
			tRecursiveMutex.lock();

			gc_dlinkptr_type* p1=p->_gc_prev;
			gc_dlinkptr_type* p2=p->_gc_next;
			p1->_gc_next=p2;
			p2->_gc_prev=p1;

			tRecursiveMutex.unlock();
		}
	}


	void gc_nolock(bool force_marking_);

	void gc(bool force_marking_=false)
	{
		tRecursiveMutex.lock();
		gc_nolock(force_marking_);
		tRecursiveMutex.unlock();
	}

	static inline void* alloc(size_t n)
	{
		return mp_alloc(n);
	}

	static inline void dealloc(void* p)
	{
		mp_free(p);
	}

	static inline void destroy(gc_obj* p)
	{
		--nNumberOfObject;
		p->~gc_obj();
		dealloc(p);
	}

	static void lock(){tRecursiveMutex.lock();}
	static void unlock(){tRecursiveMutex.unlock();}
	static uint32_t object_count(){return nNumberOfObject;}
	static uint32_t object_count_last(){return nNumberOfObjectLast;}
	static uint32_t steping_delta(){return nStepingDelta;}
	static void steping_delta(uint32_t n){nStepingDelta=n;}

	static gc_state& current();

private:

	gc_state();

	gc_mark_queue pending_q1;
	gc_mark_queue pending_q2;

	gc_dlinkptr_type tPtrLink_head;
	gc_dlinkptr_type tPtrLink_tail;

	static AtomicMutex tRecursiveMutex;
	static gc_obj* pLinkOfAllObject;

	static uint32_t nStepingDelta;
	static uint32_t nNumberOfObject;
	static uint32_t nNumberOfObjectLast;

	static gc_state* _gp_instance;
	static void _ensure_created();
};

inline gc_state& gc_state::current()
{
	if(_gp_instance==NULL)
	{
		_ensure_created();
	}
	return *_gp_instance;
}


inline void gc_mark_queue::append(gc_obj* p)
{
	if(!p||p->_gc_tags==GC_TAG_MARK) return;
	p->_gc_tags=GC_TAG_MARK;
	impl.push_back(p);
}

template<typename IT>
inline void gc_mark_queue::append(IT t1,IT t2)
{
	while(t1!=t2) append(*t1++);
}

template<typename T>
inline void gc_ptr<T>::reset(pointer p)
{
	gc_state::lock();
	m_ptr=p;
	gc_state::unlock();
}
template<typename T>
inline void gc_ptr<T>::swap(gc_ptr& o)
{
	gc_state::lock();
	std::swap(m_ptr,o.m_ptr);
	gc_state::unlock();
}

template<typename T>
inline gc_ptr<T>::gc_ptr(T* p)
{
	this->m_ptr=p;
	gc_state::current().insert(this);
}

template<typename T>
inline gc_ptr<T>::gc_ptr(const gc_ptr& o)
{
	this->m_ptr=o.m_ptr;
	gc_state::current().insert(this);
}

template<typename T>
template<typename G>
inline gc_ptr<T>::gc_ptr(const gc_ptr<G>& o)
{
	this->m_ptr=(G*)o.m_ptr;
	gc_state::insert(this);
}
template<typename T>
inline gc_ptr<T>::~gc_ptr()
{
	gc_state::current().erase(this);
}

#pragma push_macro("new")
#undef new

template<typename T>
gc_ptr<T> gc_new()
{

	T* p=(T*)gc_state::alloc(sizeof(T));
	if(!p)
	{
		System::LogTrace("gc_new: alloc failed, force collect and retry");
		gc_force_collect();

		p=(T*)gc_state::alloc(sizeof(T));
		if(!p)
		{
			System::LogTrace("gc_new failed: not enough memory");
			Exception::XBadAlloc();
		}
	}

	gc_state::lock();

	EW_ASSERT(gc_state::object_count()>=gc_state::object_count_last());

	if(gc_state::object_count()>gc_state::object_count_last()+gc_state::steping_delta())
	{
		gc_state::current().gc_nolock(false);
	}

	gc_state::addrinfo_type _tOldObjectInfo(gc_state::tCurrentObject);

	gc_state::tCurrentObject.addr=p;
	gc_state::tCurrentObject.size=sizeof(T);

	try
	{
		// constuctor may throw exception
		new(p) T();
	}
	catch(...)
	{
		gc_state::tCurrentObject=_tOldObjectInfo;
		gc_state::unlock();

		gc_state::dealloc(p);
		throw;
	}

	gc_ptr<T> gp(p);

	gc_state::tCurrentObject=_tOldObjectInfo;

	gc_state::link(p);
	gc_state::unlock();

	return gp;
}

#pragma pop_macro("new")

EW_LEAVE
#endif
