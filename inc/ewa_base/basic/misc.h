#ifndef __H_EW_BASIC_MISC__
#define __H_EW_BASIC_MISC__

#include "ewa_base/config.h"

EW_ENTER

namespace detail
{

	template<typename T>
	class buffer_part_t
	{
	public:
		typedef T* pointer;

		buffer_part_t* next;
		pointer base,gptr,wptr,last;

		size_t wr_free(){return last-wptr;}
		size_t rd_free(){return wptr-gptr;}

		void init_buff(pointer p1,pointer p2)
		{
			base=gptr=wptr=p1;
			last=p2;
		}
	};

	template<typename T>
	class buffer_part_n : public buffer_part_t<T>
	{
	public:
		static const int value=sizeof(T);
		T data[value];

		void init_size(size_t sz)
		{
			size_t nd=((sz-sizeof(*this))/sizeof(T)+value)-1;
			init_buff(&data[0],&data[nd]);
		}
	};

}

template<typename T>
class BufferPolicyBase : private NonCopyable
{
public:
	typedef T element_type;

protected:

	BufferPolicyBase(){wr_part=rd_part=NULL;}

	typedef detail::buffer_part_t<T> buffer_part;
	typedef detail::buffer_part_n<T> buffer_real;

	buffer_part* binit(buffer_part* p_,size_t sz)
	{
		buffer_real* p=(buffer_real*)p_;
		p->init_size(sz);
		if(!wr_part)
		{
			wr_part=rd_part=p;
			p->next=p;
		}
		else
		{
			p->next=wr_part->next;
			wr_part->next=p;
			wr_part=p;
		}
		return p;
	}

	static size_t btest(buffer_part* p,size_t nd)
	{
		return std::min(p->wr_free(),nd);
	}

	buffer_part* balloc(size_t)
	{
		size_t sz=1024*32;
		size_t nd=(sz-sizeof(buffer_real))/sizeof(T)+buffer_real::value;
		buffer_real* p=(buffer_real*)mp_alloc(sz);
		if(!p) Exception::XBadAlloc();
		return binit(p,sz);
	}

	buffer_part* balloc2(size_t nd)
	{
		if(!wr_part)
		{
			
		}
		else if(nd<=wr_part->wr_free())
		{
			return wr_part;
		}
		else
		{
			nd+=+wr_part->rd_free();
		}

		size_t sz=(4095+sizeof(buffer_real)+nd*sizeof(element_type))&~4095;
		buffer_real* p=(buffer_real*)mp_alloc(sz);
		if(!p) Exception::XBadAlloc();
		p->init_size(sz);

		if(wr_part)
		{
			std::copy(wr_part->gptr,wr_part->wptr,p->base);
			p->wptr=p->gptr=p->base+wr_part->rd_free();
			
			buffer_part* p2=wr_part;
			for(;p2->next!=wr_part;p2=p2->next);

			if(p2==wr_part)
			{
				rd_part=wr_part=p;
				p->next=p;	
			}
			else
			{
				p2->next=p;
				p2=wr_part;
				p->next=p2->next;
				if(rd_part==p2) rd_part=p;
				wr_part=p;
			}

			bfree(p2);
		}
		else
		{
			rd_part=wr_part=p;
			p->next=p;		
		}
		return p;
	}

	static void bfree(buffer_part* p)
	{
		mp_free(p);
	}

	buffer_part* rd_part;
	buffer_part* wr_part;

public:

	void return_initialized_buffer(size_t n)
	{
		EW_ASSERT(wr_part->wptr+n<=wr_part->last);
		wr_part->wptr+=n;
	}

	char* get_uninitialized_buffer(size_t n)
	{
		balloc2(n);
		return wr_part->wptr;
	}
};

template<typename T,int N=0>
class BufferPolicyChained : public BufferPolicyBase<T>
{
protected:

	typedef BufferPolicyBase<T> basetype;
	typedef basetype::buffer_real buffer_real;
	union
	{
		buffer_real t_data;
		char p_data[sizeof(buffer_real)+sizeof(T)*(N-buffer_real::value+1)];
	};


	BufferPolicyChained()
	{
		t_data.base=NULL;
		binit(&t_data,sizeof(p_data));
	}

	void bfree(buffer_part* p)
	{
		if(p==&t_data)
		{
			t_data.base=NULL;
		}
		else
		{
			basetype::bfree(p);
		}
	}

	buffer_part* balloc(size_t nd)
	{
		return t_data.base==NULL?binit(&t_data,sizeof(p_data)):basetype::balloc(nd);
	}

};

template<typename T>
class BufferPolicyChained<T,0> : public BufferPolicyBase<T>{};



template<typename T,int N=0>
class BufferPolicyLinear : public BufferPolicyChained<T,N>
{
protected:

	typedef BufferPolicyChained<T,N> basetype;

	size_t btest(buffer_part* p,size_t nd)
	{
		if(p->wr_free()<nd)
		{
			balloc(nd);
		}
		return nd;
	}

	buffer_part* balloc(size_t nd)
	{
		return balloc2(nd);
	}

public:

	void return_initialized_buffer(size_t n)
	{
		EW_ASSERT(wr_part->wptr+n<=wr_part->last);
		wr_part->wptr+=n;
	}

	char* get_uninitialized_buffer(size_t n)
	{
		balloc(n);
		return wr_part->wptr;		
	}

};

template<typename T,typename P=BufferPolicyChained<T> >
class MemoryBuffer : public P
{
protected:

	typedef P basetype;
	typedef T* pointer;

	using basetype::wr_part;
	using basetype::rd_part;
	using basetype::balloc;
	using basetype::bfree;

	typedef typename basetype::buffer_part buffer_part;

	void bdestroy(buffer_part* p1,buffer_part* p2)
	{
		while(p1!=p2)
		{
			buffer_part* tmp=p1;
			p1=p1->next;
			bfree(tmp);
		};
	}

public:

	MemoryBuffer(){}
	~MemoryBuffer()
	{
		if(!rd_part) return;
		bdestroy(rd_part->next,rd_part);
		bfree(rd_part);
	}

	void shrink()
	{
		if(!wr_part) return;
		if(wr_part==rd_part && rd_part->rd_free()==0)
		{
			bdestroy(rd_part->next,rd_part);
			bfree(rd_part);
			return;
		}

		for(buffer_part* p=wr_part->next;p!=rd_part && p->rd_free()==0;)
		{
			wr_part->next=p->next;
			bfree(p);
		}		
	}

	void clear()
	{
		if(!rd_part) return;
		wr_part=rd_part;
		buffer_part* p=rd_part;
		do
		{
			p->gptr=p->wptr=p->base;
			p=p->next;
		}while(p!=rd_part);
	}

	void rewind()
	{
		if(!rd_part) return;

		buffer_part* p0=rd_part;
		rd_part=wr_part->next;
		while(rd_part->gptr==rd_part->base && rd_part!=p0) rd_part=rd_part->next;

		buffer_part* p1=rd_part;
		while(1)
		{
			p1->gptr=p1->base;
			if(p1==p0) break;
			p1=p1->next;			
		};
		
	}

	int send(const T* pbuf,size_t nlen)
	{
		size_t left=nlen;
		size_t nd;

		if(!wr_part)
		{
			balloc(left);
			if(!wr_part)
			{
				return -1;
			}
		}

		while(left)
		{			
			nd=btest(wr_part,left);
			if(nd==0)
			{
				if(wr_part->next==rd_part)
				{
					balloc(left);
					continue;
				}
				else
				{
					wr_part=wr_part->next;
					wr_part->wptr=wr_part->gptr=wr_part->base;
				}
				continue;
			}

			EW_ASSERT(nd<=left);

			std::copy_n(pbuf,nd,wr_part->wptr);
			wr_part->wptr+=nd;
			pbuf+=nd;
			left-=nd;

		}

		return nlen-left;
	}

	int recv(T* pbuf,size_t nlen)
	{
		if(!rd_part) return 0;

		size_t left=nlen;
		while(left)
		{			
			size_t nd=rd_part->rd_free();
			if(nd==0)
			{
				if(rd_part==wr_part)
				{
					break;
				}
				rd_part=rd_part->next;
				continue;
			}

			size_t n1=std::min(nd,left);
			std::copy_n(rd_part->gptr,n1,pbuf);
			rd_part->gptr+=n1;
			pbuf+=n1;
			left-=n1;
		}
		return nlen-left;
	}
	
};




EW_LEAVE

#endif
