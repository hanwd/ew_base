#include "ewa_base/basic/formater.h"
#include "ewa_base/basic/string.h"
EW_ENTER



StringBuilder::operator const String&()
{
	c_str();
	return *(String*)&rd_part->gptr;		
}

const char* StringBuilder::c_str()
{
	if(rd_part!=wr_part)
	{
		size_t nd=size();

		size_t sz=(sizeof(buffer_real)+nd+4095)&~4095;

		buffer_real* r=(buffer_real*)mp_alloc(sz);
		if(!r) Exception::XBadAlloc();
		r->init_size(sz);

		recv(r->base,nd);
		r->wptr+=nd;


		bdestroy(wr_part->next,wr_part);
		bfree(wr_part);

		wr_part=rd_part=r;
		r->next=r;
	}
	wr_part->wptr[0]=0;
	return rd_part->gptr;
}

size_t StringBuilder::size()
{
	size_t nd=0;
	buffer_part* p=rd_part;
	while(1)
	{
		nd+=p->rd_free();
		if(p==wr_part) break;
		p=p->next;
	};	
	return nd;
}

void FormatState1::fmt_enter()
{
	b_fmt_ok=true;
	a_fmt_buffer.clear();
	n_fmt_width[0]=n_fmt_width[1]=0;
	fmt_append("%",1);
}

bool FormatState1::fmt_test()
{
	if(!b_fmt_ok) return false;

	f1=a_fmt_buffer.c_str();
	f2=f1+a_fmt_buffer.size();
	if(a_fmt_buffer.size()==1)
	{
		f1=f2=NULL;
	}

	return true;
}

void FormatState1::fmt_error()
{
	b_fmt_ok=false;
}

void FormatState1::fmt_leave()
{
	if(b_fmt_ok)
	{
		p1=p2;
	}
	else
	{
		n_vpos=n_vpos_old;
	}
}

template<typename S>
void FormatStateT<S>::str_append(const String& s)
{
	str_append(s.c_str());
}

template<typename S>
void FormatStateT<S>::str_append_s(const String& x)
{
	str_append(x.c_str());
}

template<typename S>
void FormatStateT<S>::str_append_s(const wchar_t* x)
{
	str_append(String(x));
}

template class FormatStateT<StringBuilder>;

void FormatState0::init(const wchar_t* p)
{
	wproxy.reset(p);
	init(wproxy.c_str());
}

void FormatState0::init(const Variant& v)
{
	wproxy.reset(v);
	init(wproxy.c_str());
}


EW_LEAVE
