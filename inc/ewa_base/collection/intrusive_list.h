#ifndef __H_EW_COLLECTION_INTRUSIVE_LIST__
#define __H_EW_COLLECTION_INTRUSIVE_LIST__

#include "ewa_base/config.h"

EW_ENTER


template<typename T>
class policy_list
{
public:

	static T* GetNext(T* pnode)
	{
		return pnode->next;
	}

	static void SetNext(T* pnode, T* next)
	{
		pnode->next=next;
	}

	static void DeleteNode(T* pnode)
	{
		delete pnode;
	}
};

template<typename T,typename P=policy_list<T> >
class intrusive_list : public NonCopyable
{
public:
	typedef T* pointer;
	typedef const T* const_pointer;

	intrusive_list()
	{
		m_pHead=m_pTail=NULL;
		m_nSize=0;
	}

	~intrusive_list()
	{
		clear();
	}

	void clear()
	{
		while(m_pHead!=NULL)
		{
			T *_tmp=m_pHead;
			m_pHead=P::GetNext(m_pHead);
			P::DeleteNode(_tmp);
		}
		m_pHead=m_pTail=NULL;
		m_nSize=0;
	}

	bool empty() const
	{
		return m_pHead==NULL;
	}

	bool remove(pointer p)
	{
		if(!m_pHead) return false;
		if(m_pHead==p)
		{
			m_pHead=P::GetNext(m_pHead);
			P::SetNext(p,NULL);
			P::DeleteNode(p);
			m_nSize--;
			return true;
		}
		pointer prev=m_pHead;
		while(prev)
		{
			pointer temp=P::GetNext(prev);
			if(temp==p)
			{
				P::SetNext(prev,P::GetNext(p));
				P::DeleteNode(p);
				m_nSize--;
				return true;
			}
			prev=temp;
		}

		return false;
	}

	bool detach(pointer p)
	{
		if(!m_pHead) return false;
		if(m_pHead==p)
		{
			m_pHead=P::GetNext(m_pHead);
			P::SetNext(p,NULL);
			m_nSize--;
			return true;
		}
		pointer prev=m_pHead;
		while(prev)
		{
			pointer temp=P::GetNext(prev);
			if(temp==p)
			{
				P::SetNext(prev,P::GetNext(p));
				m_nSize--;
				return true;
			}
			prev=temp;
		}

		return false;
	}

	void append(pointer pnode)
	{
		EW_ASSERT(pnode!=NULL);
		if(m_pTail==NULL)
		{
			m_pHead=m_pTail=pnode;
			m_nSize=1;
		}
		else
		{
			P::SetNext(m_pTail,pnode);
			m_pTail=pnode;
			m_nSize++;
		}
	}

	void insert(pointer pnode)
	{
		EW_ASSERT(pnode!=NULL);
		if(m_pHead==NULL)
		{
			m_pHead=m_pTail=pnode;
			m_nSize=1;
		}
		else
		{
			P::SetNext(pnode,m_pHead);
			m_pHead=pnode;
			m_nSize++;
		}
	}

	pointer pop_front()
	{
		if(!m_pHead) return NULL;
		pointer q=m_pHead;
		m_pHead=P::GetNext(q);
		if(!m_pHead)
		{
			m_pTail=NULL;
		}
		m_nSize--;
		return q;
	}

	size_t size() const
	{
		return m_nSize;
	}

	pointer head()
	{
		return m_pHead;
	}

	pointer tail()
	{
		return m_pTail;
	}

	const_pointer head() const
	{
		return m_pHead;
	}

	const_pointer tail() const
	{
		return m_pTail;
	}

	class iterator
	{
	public:
		iterator(pointer p=NULL):m_ptr(p) {}

		iterator &operator++() const
		{
			if(m_ptr)
			{
				m_ptr=P::GetNext(m_ptr);
			}
			return *this;
		}

		iterator operator++(int) const
		{
			iterator old(m_ptr);
			if(m_ptr)
			{
				m_ptr=P::GetNext(m_ptr);
			}
			return old;
		}

		T &operator*()
		{
			EW_ASSERT(!m_ptr);
			return *m_ptr;
		}

		const T &operator*() const
		{
			EW_ASSERT(!m_ptr);
			return *m_ptr;
		}

		bool operator==(const iterator it) const
		{
			return m_ptr==it.m_ptr;
		}

		bool operator!=(const iterator it) const
		{
			return m_ptr!=it.m_ptr;
		}

	private:
		pointer m_ptr;
	};

	typedef const iterator const_iterator;

	iterator begin()
	{
		return m_pHead;
	}

	iterator end()
	{
		return NULL;
	}

	const_iterator begin() const
	{
		return m_pHead;
	}

	const_iterator end() const
	{
		return NULL;
	}

	void release()
	{
		m_pHead=m_pTail=NULL;
		m_nSize=0;
	}

	void swap(intrusive_list &o)
	{
		std::swap(m_pHead,o.m_pHead);
		std::swap(m_pTail,o.m_pTail);
		std::swap(m_nSize,o.m_nSize);
	}

protected:
	pointer m_pHead;
	pointer m_pTail;
	size_t m_nSize;
};

EW_LEAVE

#endif
