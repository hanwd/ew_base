#ifndef __H_EW_COLLECTION_LINEAR_BUFFER__
#define __H_EW_COLLECTION_LINEAR_BUFFER__

#include <complex>

#include "ewa_base/collection/arr_1t.h"
#include "ewa_base/collection/arr_xt.h"
#include "ewa_base/basic/atomic.h"
//#include "ewa_base/basic/system.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/logging/logger.h"

EW_ENTER

// RingBufferBase one thread read/one thread write
template<typename T>
class DLLIMPEXP_EWA_BASE LinearBuffer : private NonCopyable
{
public:
	typedef size_t size_type;

	LinearBuffer()
	{
		pBuffer=NULL;
		wr_pos=rd_pos=0;
		sz_buf=0;
	}

	void rewind()
	{
		wr_pos=rd_pos=0;
	}

	// rewind and reserve buffer of size_
	void rewind(size_type size_)
	{
		aBuff.resize(size_);
		_xbuf();
	}

	bool skip()
	{
		bool flag=wr_pos==rd_pos;
		wr_pos=rd_pos=0;
		return flag;
	}

	bool eof()
	{
		return rd_pos==wr_pos;
	}

	T get()
	{
		if(eof()) return -1;
		return pBuffer[rd_pos++];
	}

	T peek()
	{
		if(eof()) return -1;
		return pBuffer[rd_pos];
	}

	void unget()
	{
		if(rd_pos==0)
		{
			Exception::XInvalidIndex();
			return;
		}
		--rd_pos;
	}

	void wr_flip(size_type n)
	{
		wr_pos+=n;
	}

	void rd_flip(size_type n)
	{
		rd_pos+=n;
	}

	void rd_reset()
	{
		rd_pos=0;
	}

	size_type peek(T* p_,size_type n)
	{
		size_type kp=wr_pos-rd_pos;
		if(kp>n)
		{
			kp=n;
		}
		memcpy(p_,pBuffer+rd_pos,kp);
		return kp;
	}

	size_type rd_free()
	{
		return wr_pos-rd_pos;
	}

	size_type wr_free()
	{
		return sz_buf-wr_pos;
	}

	size_type send(const T* p_,size_type n)
	{
		size_type wr_max=wr_pos+n;
		if(wr_max>sz_buf && !_grow(wr_max))
		{
			return 0;
		}

		memcpy(pBuffer+wr_pos,p_,n);
		wr_pos+=n;
		return n;
	}

	size_type recv(T* p_,size_type n)
	{
		size_type kp=wr_pos-rd_pos;

		if(kp>n)
		{
			kp=n;
		}
		memcpy(p_,pBuffer+rd_pos,kp);
		rd_pos+=kp;

		return kp;
	}

	T* gbeg()
	{
		return pBuffer;   // buffer begin
	}
	T* gptr()
	{
		return pBuffer+rd_pos;   // get position begin
	}
	T* gend()
	{
		return pBuffer+wr_pos;   // get position end or put position begin
	}
	T* last()
	{
		return pBuffer+sz_buf;
	}

	// assign external buffer
	void assign(T* pbuf_,size_t size_)
	{
		pBuffer=(T*)pbuf_;
		sz_buf=size_;
		rd_pos=wr_pos=0;
	}

	// allocate buffer
	void alloc(size_type size_)
	{
		aBuff.resize(size_);
		aBuff.shrink_to_fit();
		_xbuf();
	}

	bool load(const String& file,int t=FILE_TEXT)
	{
		if(!aBuff.load(file,t)) return false;
		_xbuf();
		wr_pos=sz_buf;
		return true;
	}


	template<typename E,typename A>
	bool parse(arr_1t<E,A>& a1t,int64_t* pn=NULL);

	template<typename E,typename A>
	bool parse(arr_xt<E,A>& axt);

protected:

	T* pBuffer;
	size_type sz_buf; // buffer size
	size_type rd_pos; // get position begin
	size_type wr_pos; // get position end or put position begin
	StringBuffer<T> aBuff;

	bool _grow(size_type _newsize);

	void _xbuf()
	{
		pBuffer=aBuff.data();
		sz_buf=aBuff.size();
		rd_pos=wr_pos=0;
	}

};



template<typename T>
void skip_space(LinearBuffer<T>& lbuf)
{
	for(;;)
	{
		if(lbuf.eof()) return;
		T val=lbuf.get();
		if(val!='\t' && val!=' ')
		{
			lbuf.unget();
			return;
		}
	}
};

template<typename T>
void skip_comment1(LinearBuffer<T>& lbuf) // comment //
{
	lbuf.rd_flip(1);

	if(lbuf.get()!='/')
	{
		Exception::XError("'/' expected",false);
		return;
	}

	for(;;)
	{
		if(lbuf.eof()) return;
		T val=lbuf.get();

		if(val=='\r'||val=='\n')
		{
			lbuf.unget();
			return;
		}
	}
};

template<typename T>
void skip_comment2(LinearBuffer<T>& lbuf) // comment %
{
	lbuf.rd_flip(1);
	for(;;)
	{
		if(lbuf.eof()) return;
		T val=lbuf.get();

		if(val=='\r'||val=='\n')
		{
			lbuf.unget();
			return;
		}
	}
};


template<typename T>
void read_integer(LinearBuffer<T>& lbuf,int64_t& vv,int& gg)
{
	vv=0;
	gg=0;
	int kk=0;

	int doted=0;

	bool sign1=false;

	T val=lbuf.peek();
	if(val=='+') lbuf.get();
	if(val=='-')
	{
		sign1=true;
		lbuf.get();
	}

	for(;;)
	{
		if(lbuf.eof())
		{
			return;
		}

		T val=lbuf.get();
		unsigned d=val-'0';
		if(d>10)
		{
			if(val=='.')
			{
				if(doted==1)
				{
					Exception::XError("Invalid_number:two dot",false);
				}

				doted=1;
				continue;
			}

			lbuf.unget();
			break;
		}

		kk++;
		gg=gg+doted;
		vv=vv*10+d;
	}

	if(kk==0)
	{
		Exception::XError("Invalid_number",false);
	}

	if(sign1)
	{
		vv=-vv;
	}
}

template<typename T,typename E>
void read_element(LinearBuffer<T>& lbuf,E& v)
{
	int64_t p1,p2;
	int s1,s2;

	read_integer(lbuf,p1,s1);

	T val=lbuf.peek();
	if(val=='d'||val=='D'||val=='e'||val=='E')
	{
		lbuf.get();
		read_integer(lbuf,p2,s2);
		if(s2!=0)
		{
			Exception::XError("Invalid_number:exponent with dot",false);
		}
		s1=p2-s1;
	}
	else
	{
		s1=-s1;
	}

	v=(E)(float64_t(p1)*::pow(10.0,s1));

}


template<typename T>
void read_element(LinearBuffer<T>& lbuf,String& v)
{
	T* p1=lbuf.gptr();

	for(;;)
	{
		if(lbuf.eof())
		{
			break;
		}

		T val=lbuf.peek();
		if(val=='\0'||val=='\t'||val==' '||val=='\r'||val=='\n')
		{
			break;
		}

		lbuf.get();
	}
	T* p2=lbuf.gptr();

	StringBuffer<T> sb(p1,p2);
	v=sb;
}

template<typename T,typename E>
void read_element(LinearBuffer<T>& lbuf,std::complex<E>& v)
{
	T val=lbuf.peek();
	E _re,_im;

	if(val=='(')
	{
		lbuf.get();
		skip_space(lbuf);
		read_element(lbuf,_re);
		skip_space(lbuf);
		val=lbuf.get();
		if(val!=',')
		{
			lbuf.unget();
			Exception::XError("unexpecter_character",false);
		}
		skip_space(lbuf);
		read_element(lbuf,_im);
		skip_space(lbuf);
		val=lbuf.get();

		if(val!=')')
		{
			lbuf.unget();
			Exception::XError("unexpecter_character",false);
		}
	}
	else
	{
		read_element(lbuf,_re);
		skip_space(lbuf);
		read_element(lbuf,_im);
		T val=lbuf.get();
		if(val!='i'&&val!='j')
		{
			lbuf.unget();
			Exception::XError("Invalid_complex",false);
		}
	}

	v=std::complex<E>(_re,_im);

}


template<typename T>
template<typename E,typename A>
bool LinearBuffer<T>::parse(arr_1t<E,A>& a1t,int64_t* pn)
{
	rd_reset();
	if(wr_pos==0)
	{
		a1t.clear();
		return true;
	}

	arr_1t<E,A> tmp;
	int64_t ln=0;

	try
	{
		int64_t tn=0;
		E el;

		for(;;)
		{
			if(eof())
			{
				break;
			}

			T val=peek();
			if(val=='/')
			{
				skip_comment1(*this);
			}
			else if(val=='%')
			{
				skip_comment2(*this);
			}
			else if(val=='\t'||val==' ')
			{
				skip_space(*this);
			}
			else if(val=='\r'||val=='\n')
			{
				get();
				if(tn==0) continue;
				if(ln==0)
				{
					ln=tn;
				}
				else if(ln!=tn)
				{
					ln=-1;
				}
				tn=0;
			}
			else
			{
				read_element(*this,el);
				tmp.push_back(el);
				tn=tn+1;
			}
		}
	}
	catch(std::exception& e)
	{
		this_logger().LogError("LinearBuffer<T>::parse:%s",e.what());
		return false;
	}

	if(pn) *pn=ln;
	a1t.swap(tmp);
	return true;
}

template<typename T>
template<typename E,typename A>
bool LinearBuffer<T>::parse(arr_xt<E,A>& axt)
{
	arr_1t<E,A> a1t;
	int64_t sz2;

	if(!parse(a1t,&sz2))
	{
		return false;
	}

	if(sz2<=0)
	{
		this_logger().LogError("LinearBuffer<T>::parse:row_size_mismatch");
		return false;
	}

	if(a1t.size()%sz2!=0)
	{
		this_logger().LogError("LinearBuffer<T>::parse:invalid_element_count");
		return false;
	}

	int64_t sz1=a1t.size()/sz2;
	axt.resize(sz2,sz1);
	xmem<E>::copy(a1t.begin(),a1t.end(),axt.begin());
	return true;
}

EW_LEAVE

#endif
