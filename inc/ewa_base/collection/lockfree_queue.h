#ifndef __H_EW_COLLECTION_LOCKFREE_QUEUE__
#define __H_EW_COLLECTION_LOCKFREE_QUEUE__

#include "ewa_base/collection/detail/collection_base.h"
#include "ewa_base/basic/atomic.h"
#include "ewa_base/basic/bitflags.h"
#include "ewa_base/basic/exception.h"

#include <cstdlib>


EW_ENTER

template<typename T>
class LockFreeQueuePolicy
{
public:
	typedef uint32_t size_type;

	// if getq/putq spin count > timeout, and queue timeout flag is set, current thread will enter.
	// this is designed to avoid that some thread get the key but failed to give back.
	static inline size_type timeout()
	{
		return 1024*1024*16;
	}

	// while Queue is NonBlock, getq return invalid_value() if Queue is empty.
	// alternative implementation: throw an exception
	static inline T invalid_value()
	{
		return T();
	}

	// move value from b to a.
	// alternative implementation: swap(a,b);
	static inline void move_value(T& a,T& b)
	{
		a=b;
	}

	static inline void copy_n(T* a,T* b,size_t n)
	{
		memcpy(a,b,sizeof(T)*n);
	}

	static inline void move_n(T* a,T* b,size_t n)
	{
		memcpy(a,b,sizeof(T)*n);
	}

	// if T is not POD, fill/destroy method must be implemented.
	static inline void fill(T*,size_type) {}
	static inline void destroy(T*,size_type) {}

	// called when key is busy or condition not met, such as queue is full while putq.
	// alternative implementation: Thread::yield()
	static inline void noop() {}

};

template<typename T>
class LockFreeQueuePolicyInt : public LockFreeQueuePolicy<T>
{
public:
	static inline T invalid_value()
	{
		return T(-1);
	}
};

template<typename T>
class LockFreeQueuePolicyObj : public LockFreeQueuePolicy<T>
{
public:
	typedef typename LockFreeQueuePolicy<T>::size_type size_type;
	static inline void fill(T* p,size_type n)
	{
		xmem<T>::uninitialized_fill_n(p,n,T());
	}
	static inline void destroy(T* p,size_type n)
	{
		xmem<T>::destroy(p,n);
	}
};

class lf_queue_header
{
public:
	AtomicUint32 head;
	AtomicUint32 rear;
	AtomicInt32 kget;
	AtomicInt32 kput;

	uint32_t mask;
	BitFlags flags;

};

// T must be pod_data
template<typename T,typename P=LockFreeQueuePolicy<T> >
class LockFreeQueue : private NonCopyable
{
public:

	typedef typename P::size_type size_type;

	enum
	{
		// queue is intialized
		QUEUE_INITED	=1<<0,

		// queue is nonblock
		QUEUE_NONBLOCK	=1<<1,

		// get key and enter if timeout
		QUEUE_TIMEOUT	=1<<2,


		QUEUE_PARTIAL	=1<<3,

		QUEUE_FREE_HEADER=1<<4,
		QUEUE_FREE_BUFFER=1<<5,
	};

	static T invalid_value()
	{
		return P::invalid_value();
	}


	// initialized?
	bool ok() const
	{
		return pHeader!=NULL;
	}

	// return queue size.
	size_type size() const
	{
		if(!pHeader) return 0;
		return (pHeader->rear.get()-pHeader->head.get())&pHeader->mask;
	}

	// return queue capacity, the queue actually can hold capacity-1 elements.
	size_type capacity() const
	{
		if(!pHeader) return 0;
		return pHeader->mask+1;
	}

	bool empty() const
	{
		if(!pHeader) return false;
		return pHeader->rear.get()==pHeader->head.get();
	}

	bool full() const
	{
		if(!pHeader) return true;
		return ((pHeader->rear.get()+1)&pHeader->mask)==pHeader->head.get();
	}

	// queue flags
	BitFlags& flags()
	{
		if(!pHeader)
		{
			System::LogTrace("pHeader==NULL while calling LockFreeQueue::flags");
			static BitFlags f;
			return f;
		}
		return pHeader->flags;
	}

	// get an element from queue head.
	T getq();

	int getq(T* p,int n);

	int peek(T* p,int n);

	// push an element to the queue rear
	bool putq(T v);

	// push n elements to the queue rear
	int putq(T* p,int n);

	void rewind();

	void clear();

	// create a lockfree queue with capacity of v
	// v will be adjust to 2^n
	void reset(size_type v);

	// initialize the queue with give addr and size.
	bool init(lf_queue_header* addr_,T* buff_,bool forceinit_);

	LockFreeQueue()
	{
		pHeader=NULL;
		pBuffer=NULL;
	}

	LockFreeQueue(size_type n,int f)
	{
		pHeader=NULL;
		pBuffer=NULL;
		reset(n);
		flags().add(f);
	}

	~LockFreeQueue()
	{
		clear();
	}

private:
	lf_queue_header* pHeader;
	T* pBuffer;
};


// single read single write queue
template<typename T>
class SRSW_Queue
{

	typedef void (*buf_free_function)(void*);

	class SRSW_queue_impl
	{
	public:
		buf_free_function dfun;
		AtomicIntT<intptr_t> nref;
		size_t size;
		volatile size_t rd_pos;
		volatile size_t wr_pos;
		T data[1];

		bool push(const T& t)
		{
			size_t wr2=(wr_pos+1)%size;
			if(wr2==rd_pos)
			{
				return false;
			}
			data[wr_pos]=t;
			wr_pos=wr2;
			return true;
		}

		bool popq(T& t)
		{
			if(rd_pos==wr_pos)
			{
				return false;
			}
			t=data[rd_pos];
			rd_pos=(rd_pos+1)%size;
			return true;
		}
	};


public:

	SRSW_Queue()
	{
		m_ptr=NULL;
	}

	SRSW_Queue(const SRSW_Queue& q)
	{
		m_ptr=NULL;
		reset(q.m_ptr);
	}

	SRSW_Queue& operator=(const SRSW_Queue& q)
	{
		reset(q.m_ptr);
		return *this;
	}

	~SRSW_Queue()
	{
		clear();
	}

	void clear()
	{
		reset(NULL);
	}

	void reset(SRSW_queue_impl* p)
	{
		if(p)
		{
			++p->nref;
		}

		if(m_ptr && --m_ptr->nref==0)
		{
			if(m_ptr->dfun)
			{
				m_ptr->dfun(m_ptr);
			}
		}

		m_ptr=p;
	}

	bool reset(void* b,size_t d,buf_free_function f=NULL)
	{
		if(d<sizeof(SRSW_queue_impl))
		{
			return false;
		}
		size_t n=1+(d-sizeof(SRSW_queue_impl))/sizeof(T);
		SRSW_queue_impl* p=(SRSW_queue_impl*)b;

		p->dfun=f;
		p->nref=0;
		p->size=n;
		p->wr_pos=0;
		p->rd_pos=0;

		reset(p);

		return true;
	}

	void resize(size_t n)
	{
		if(n>0)
		{
			size_t sz=sizeof(SRSW_queue_impl)+sizeof(T)*(n-1);
			if((sz-sizeof(SRSW_queue_impl))/sizeof(T)!=n-1) Exception::XBadAlloc();
			void* p=mp_alloc(sz);
			if(!p) Exception::XBadAlloc();
			reset(p,sz,mp_free);
		}
		else
		{
			reset(NULL);
		}
	}

	bool push(const T& t)
	{
		EW_ASSERT(m_ptr!=NULL);
		return m_ptr->push(t);
	}

	bool popq(T& t)
	{
		EW_ASSERT(m_ptr!=NULL);
		return m_ptr->popq(t);
	}

	SRSW_queue_impl* impl(){return m_ptr;}

protected:

	SRSW_queue_impl* m_ptr;
};


template<typename T,typename P>
T LockFreeQueue<T,P>::getq()
{
	EW_ASSERT(pHeader!=NULL);

	T q;
	int32_t tag;

	// test and check the key
	// if original value is zero then enter and get an element, and then store zero to the key and leave
	while((tag=pHeader->kget.fetch_add(1))!=0)
	{
		if(tag!=(int32_t)P::timeout())
		{
			P::noop();
			continue;
		}

		if(pHeader->kget.exchange(1)==0)
		{
			break;
		}

		if(pHeader->flags.get(QUEUE_TIMEOUT)) // timeout
		{
			System::LogTrace("getq key timeout!");
			break;
		}
	}

	while(pHeader->head.get()==pHeader->rear.get()) // the queue is empty
	{
		if(pHeader->flags.get(QUEUE_NONBLOCK)) // return invalid_value if nonblock
		{
			pHeader->kget.store(0);
			return P::invalid_value();
		}
		else
		{
			// store(1) to tell others that I'm alive.
			pHeader->kget.store(1);
			P::noop();
		}
	}

	P::move_value(q,pBuffer[pHeader->head.get()]); // get an element from head
	pHeader->head.store((pHeader->head.get()+1)&pHeader->mask); // advance head position
	pHeader->kget.store(0); // give back the key.

	return q;
}

template<typename T,typename P>
int LockFreeQueue<T,P>::getq(T* p,int n)
{
	EW_ASSERT(pHeader!=NULL);
	int32_t tag;

	// test and check the key
	// if original value is zero then enter and get an element, and then store zero to the key and leave
	while((tag=pHeader->kget.fetch_add(1))!=0)
	{
		if(tag!=(int32_t)P::timeout())
		{
			P::noop();
			continue;
		}

		if(pHeader->kget.exchange(1)==0)
		{
			break;
		}

		if(pHeader->flags.get(QUEUE_TIMEOUT)) // timeout
		{
			System::LogTrace("getq key timeout!");
			break;
		}
	}


	while(pHeader->head.get()==pHeader->rear.get()) // the queue is empty
	{
		if(pHeader->flags.get(QUEUE_NONBLOCK))
		{
			pHeader->kget.store(0);
			return 0;
		}
		else
		{
			// store(1) to tell others that I'm alive.
			pHeader->kget.store(1);
			P::noop();
		}
	}

	int rd=pHeader->head.get();
	int wr=pHeader->rear.get();
	int kp=(wr-rd)&pHeader->mask;
	if(kp>n)
	{
		kp=n;
	}
	int kn=rd+kp-pHeader->mask-1;

	if(kn>0)
	{
		P::move_n(p,pBuffer+rd,kp-kn);
		P::move_n(p+kp-kn,pBuffer,kn);
		pHeader->head.store(kn);
	}
	else
	{
		P::move_n(p,pBuffer+rd,kp);
		pHeader->head.store(kn&pHeader->mask);
	}

	pHeader->kget.store(0); // give back the key.
	return kp;
}

template<typename T,typename P>
int LockFreeQueue<T,P>::peek(T* p,int n)
{
	EW_ASSERT(pHeader!=NULL);
	int32_t tag;

	// test and check the key
	// if original value is zero then enter and get an element, and then store zero to the key and leave
	while((tag=pHeader->kget.fetch_add(1))!=0)
	{
		if(tag!=(int32_t)P::timeout())
		{
			P::noop();
			continue;
		}

		if(pHeader->kget.exchange(1)==0)
		{
			break;
		}

		if(pHeader->flags.get(QUEUE_TIMEOUT)) // timeout
		{
			System::LogTrace("getq key timeout!");
			break;
		}
	}

	while(pHeader->head.get()==pHeader->rear.get()) // the queue is empty
	{
		if(pHeader->flags.get(QUEUE_NONBLOCK))
		{
			pHeader->kget.store(0);
			return 0;
		}
		else
		{
			// store(1) to tell others that I'm alive.
			pHeader->kget.store(1);
			P::noop();
		}
	}

	int rd=pHeader->head.get();
	int wr=pHeader->rear.get();
	int kp=(wr-rd)&pHeader->mask;
	if(kp>n)
	{
		kp=n;
	}
	int kn=rd+kp-pHeader->mask-1;

	if(kn>0)
	{
		P::copy_n(p,pBuffer+rd,kp-kn);
		P::copy_n(p+kp-kn,pBuffer,kn);
		pHeader->head.store(kn);
	}
	else
	{
		P::copy_n(p,pBuffer+rd,kp);
		pHeader->head.store(kn&pHeader->mask);
	}

	pHeader->kget.store(0); // give back the key.
	return kp;
}

template<typename T,typename P>
bool LockFreeQueue<T,P>::putq(T v)
{
	EW_ASSERT(pHeader!=NULL);

	int32_t tag;

	// test and check the key
	// if original value is zero then enter and get an element, and then store zero to the key and leave
	while((tag=pHeader->kput.fetch_add(1))!=0)
	{
		if(tag!=(int32_t)P::timeout())
		{
			P::noop();
			continue;
		}

		if(pHeader->kput.exchange(1)==0)
		{
			break;
		}

		if(pHeader->flags.get(QUEUE_TIMEOUT))
		{
			System::LogTrace("getq key timeout!");
			break;
		}
	}

	while(((pHeader->rear.get()+1)&pHeader->mask)==pHeader->head.get()) // queue is full
	{
		if(pHeader->flags.get(QUEUE_NONBLOCK)) // give back the key and return false if nonblock
		{
			pHeader->kput.store(0);
			return false;
		}
		else
		{
			pHeader->kput.store(1); // store(1) to tell others that I'm alive.
			P::noop();
		}
	}

	P::move_value(pBuffer[pHeader->rear.get()],v); // enqueue an element
	pHeader->rear.store((pHeader->rear.get()+1)&pHeader->mask); // advance rear position
	pHeader->kput.store(0); // give back the key.
	return true;
}

template<typename T,typename P>
int LockFreeQueue<T,P>::putq(T* p,int n)
{
	EW_ASSERT(pHeader!=NULL);

	int32_t tag;

	// test and check the key
	// if original value is zero then enter and get an element, and then store zero to the key and leave
	while((tag=pHeader->kput.fetch_add(1))!=0)
	{
		if(tag!=(int32_t)P::timeout())
		{
			P::noop();
			continue;
		}

		if(pHeader->kput.exchange(1)==0)
		{
			break;
		}

		if(pHeader->flags.get(QUEUE_TIMEOUT))
		{
			System::LogTrace("getq key timeout!");
			break;
		}
	}

	while(((pHeader->rear.get()+1)&pHeader->mask)==pHeader->head.get()) // queue is full
	{
		if(pHeader->flags.get(QUEUE_NONBLOCK)) // give back the key and return false if nonblock
		{
			pHeader->kput.store(0);
			return 0;
		}
		else
		{
			pHeader->kput.store(1); // store(1) to tell others that I'm alive.
			P::noop();
		}
	}

	int rd=pHeader->head.get();
	int wr=pHeader->rear.get();
	int kp=(rd-wr-1)&pHeader->mask;

	if(kp>=n)
	{
		kp=n;
	}
	else if(!pHeader->flags.get(QUEUE_PARTIAL))
	{
		return 0;
	}

	int kn=wr+kp-pHeader->mask-1;
	if(kn>0)
	{
		P::copy_n(pBuffer+wr,p,kp-kn);
		P::copy_n(pBuffer,p+kp-kn,kn);
		pHeader->rear.store(kn);
	}
	else
	{
		P::copy_n(pBuffer+wr,p,kp);
		pHeader->rear.store(kn&pHeader->mask);
	}

	pHeader->kput.store(0); // give back the key.
	return kp;
}
template<typename T,typename P>
void LockFreeQueue<T,P>::rewind()
{
	if(!pHeader)
	{
		return;
	}

	int32_t tag;

	while((tag=pHeader->kput.fetch_add(1))!=0)
	{
		P::noop();
	}
	while((tag=pHeader->kget.fetch_add(1))!=0)
	{
		pHeader->kput.store(0);
		//Thread::yield();
		while((tag=pHeader->kput.fetch_add(1))!=0)
		{
			P::noop();
		}
	}

	pHeader->head.store(0);
	pHeader->rear.store(0);
	pHeader->kput.store(0);
	pHeader->kget.store(0);


}
template<typename T,typename P>
void LockFreeQueue<T,P>::clear()
{
	if(!pHeader)
	{
		return;
	}

	if(pHeader->flags.get(QUEUE_FREE_BUFFER))
	{
		P::destroy(pBuffer,pHeader->mask+1);
		::free(pBuffer);
	}

	if(pHeader->flags.get(QUEUE_FREE_HEADER))
	{
		::free(pHeader);
	}

	pBuffer=NULL;
	pHeader=NULL;
}

template<typename T,typename P>
void LockFreeQueue<T,P>::reset(size_type v)
{
	v=sz_helper::n2p(v);

	lf_queue_header* p1=(lf_queue_header*)::malloc(sizeof(lf_queue_header));
	if(!p1)
	{
		Exception::XBadAlloc();
	}
	T* p2=(T*)::malloc(sizeof(T)*v);

	if(p2==NULL)
	{
		::free(p2);
		Exception::XBadAlloc();
	}

	p1->flags.clr(QUEUE_FREE_BUFFER|QUEUE_FREE_HEADER);
	p1->mask=v-1;
	init(p1,p2,true);
}

template<typename T,typename P>
bool LockFreeQueue<T,P>::init(lf_queue_header* addr_,T* buff_,bool forceinit_)
{
	//size_type _nElem=addr_->mask+1;
	clear();

	pHeader=addr_;
	pBuffer=buff_;

	if(!forceinit_)
	{
		return true;
	}

	pHeader->flags.add(QUEUE_INITED);
	pHeader->head.store(0);
	pHeader->rear.store(0);
	pHeader->kget.store(0);
	pHeader->kput.store(0);

	P::fill(pBuffer,pHeader->mask+1);

	return true;

}



EW_LEAVE


#endif

