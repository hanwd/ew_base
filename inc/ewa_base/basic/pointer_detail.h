#ifndef __H_EW_POINTER_DETAIL__
#define __H_EW_POINTER_DETAIL__

#include "ewa_base/basic/object.h"

#ifdef _MSC_VER
#pragma warning(disable:4180)
#endif

EW_ENTER


namespace detail
{

	template<typename T>
	class po_lite
	{
	public:

		static inline void reset(T* &p1,T* p2)
		{
			p1=p2;
		}

		static inline void equal(T* &p1,T* p2)
		{
			p1=p2;
		}
	};

	template<typename T>
	class po_auto
	{
	public:
		static inline void reset(T* &p1,T* p2)
		{
			if(p1==p2) return;
			delete p1;
			p1=p2;
		}

	private:
		static void equal(T* &p1,T* p2);
	};

	template<typename T>
	class po_data
	{
	public:

		static inline void reset(T* &p1,T* &p2)
		{
			if(p1==p2) return;
			if(p2) p2->IncRef();
			if(p1) p1->DecRef();
			p1=p2;
		}

		static inline void equal(T* &p1,T*& p2)
		{
			if(p1==p2) return;
			if(p2) p2->IncRef();
			if(p1) p1->DecRef();
			p1=p2;
		}

	};

	template<typename T>
	class po_sel
	{
	public:
		static const bool value=tl::is_convertible<T,ObjectData>::value;
		typedef typename tl::meta_if<value,po_data<T>,po_auto<T> >::type type;

	};

	template<typename T,typename P>
	class BasePtrT
	{
	public:
		typedef T *pointer;

		inline BasePtrT(T *p=NULL)
		{
			m_ptr=NULL;
			reset(p);
		}

		inline const BasePtrT &operator=(const BasePtrT& p)
		{
			P::equal(m_ptr,const_cast<T*&>(p.m_ptr));
			return *this;
		}

		inline BasePtrT(const BasePtrT& p) :m_ptr(NULL)
		{
			P::equal(m_ptr,const_cast<T*&>(p.m_ptr));
		}

		inline ~BasePtrT()
		{
			reset(NULL);
		}

		inline void swap(BasePtrT& p)
		{
			std::swap(m_ptr,p.m_ptr);
		}

		inline void reset(pointer p)
		{
			P::reset(m_ptr,p);
		}

		template<typename X,typename Y>
		inline typename tl::enable_cv<X,T>::type reset(BasePtrT<X,Y> p)
		{
			P::reset(m_ptr,p.get());
		}

		inline pointer get()
		{
			return m_ptr;
		}

		inline const T* get() const
		{
			return m_ptr;
		}

		inline T &operator*()
		{
			EW_ASSERT(m_ptr!=NULL);
			return *m_ptr;
		}

		inline const T &operator*() const
		{
			EW_ASSERT(m_ptr!=NULL);
			return *m_ptr;
		}

		inline pointer operator->()
		{
			EW_ASSERT(m_ptr!=NULL);
			return m_ptr;
		}

		inline const T* operator->() const
		{
			EW_ASSERT(m_ptr!=NULL);
			return m_ptr;
		}

		inline operator bool() const
		{
			return m_ptr!=NULL;
		}

		inline bool operator<(const BasePtrT& o)
		{
			return m_ptr<o.m_ptr;
		}


	protected:
		T *m_ptr;
	};
};

EW_LEAVE
#endif
