
#include "ewa_base/basic/codecvt.h"
#include "ewa_base/basic/system.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/logging/logger.h"

EW_ENTER

//byte[0] 0x81~0xFE;
//byte[1] 0x40~0x7E,0x80~0xFE
static const unsigned gbk_ke1=(0xFE)-(0x81)+1;
static const unsigned gbk_ke2=(0xFE)-(0x40);

extern uint16_t g_gkb_table[];
extern uint16_t g_uni_table[1024*64];

void GBKTable_init()
{
	if(g_uni_table[0]!=0) return;
	for(size_t i=0; i<gbk_ke1*gbk_ke2; i++)
	{
		size_t b1=(i/gbk_ke2)+0x81;
		size_t b2=(i%gbk_ke2)+0x40;
		if(b2>0x7E) b2=b2+1;
		uint16_t bp=b1+b2*256;

		g_uni_table[g_gkb_table[i]]=bp;
	}
}




template<typename WC>
bool IConv_unicode_to_gbk(StringBuffer<uint8_t>& aa_,const WC* pw_,size_t ln_)
{
	GBKTable_init();

	StringBuffer<uint8_t> dst;
	dst.resize(ln_*2);

	uint8_t* pa_=dst.data();
	const WC* pw2=pw_+ln_;

	int _n_invalid_count=0;

	for(; pw_<pw2; pw_++)
	{
		if(pw_[0]<0x81)
		{
			*pa_++=pw_[0];
			continue;
		}

		if(sizeof(WC)>2&&(pw_[0]&0xFFFF0000)!=0)
		{
			_n_invalid_count++;
			System::LogWarning("IConv_unicode_to_gbk: unkown unicode character %x",(uint32_t)pw_[0]);

			pa_[0]='?';
			pa_[1]='?';
			pa_+=2;
			continue;
		}

		WC val=g_uni_table[pw_[0]];
		if(val!=0)
		{
			pa_[0]=(((uint8_t*)&val)[0]);
			pa_[1]=(((uint8_t*)&val)[1]);
			pa_+=2;
		}
		else
		{
			*pa_++=pw_[0]&0x7F;
		}
	}
	size_t la_=pa_-dst.data();
	dst.resize(la_);
	dst.swap(aa_);

	return _n_invalid_count==0;
}

bool IConv::unicode_to_gbk(StringBuffer<uint8_t>& aa_,const uint16_t* pw_,size_t ln_)
{
	return IConv_unicode_to_gbk<uint16_t>(aa_,pw_,ln_);
}

bool IConv::unicode_to_gbk(StringBuffer<uint8_t>& aa_,const uint32_t* pw_,size_t ln_)
{
	return IConv_unicode_to_gbk<uint32_t>(aa_,pw_,ln_);
}


String IConv::from_unknown(const char* s)
{
	uint8_t* p=(uint8_t*)s;
	size_t nz=::strlen(s);

	int t=0;
	int n=0;

	for(size_t i=0; i<nz; i++)
	{
		unsigned c=p[i];
		if(c<0x80) continue;
		for(n=0; ((c<<=1)&0x80)>0; n++);
		if(n>3||n==0)
		{
			t=-1;
			break;
		}

		if(i+n>=nz)
		{
			t=-1;
			break;
		}

		t=1;
		for(int j=1; j<=n; j++)
		{
			if(((p[i+j])&0xC0)!=0x80)
			{
				t=-1;
				break;
			}
		}
		if(t==-1) break;

		i+=n;

		if(t==-1) break;
	}

	if(t==-1)
	{
		return from_gbk(s);
	}
	else
	{
		return s;
	}

}

template<typename WC>
bool IConv_gbk_to_unicode(StringBuffer<WC>& aw_,const uint8_t* pa_,size_t ln_)
{
	StringBuffer<WC> dst;
	dst.resize(ln_);
	WC* pw_=dst.data();

	const uint8_t *pa2=pa_+ln_;

	while(pa_<pa2)
	{
		unsigned b1=pa_[0]-0x81;
		if(b1>=gbk_ke1)
		{
			*pw_++=*pa_++;
			continue;
		}

		uint8_t ac=pa_[1];
		if(ac>=0x40&&ac<=0x7E)
		{
			unsigned b2=ac-0x40;
			*pw_++=g_gkb_table[b1*gbk_ke2+b2];
			pa_+=2;
		}
		else if(ac>=0x80&&ac<=0xFE)
		{
			unsigned b2=ac-0x41;
			*pw_++=g_gkb_table[b1*gbk_ke2+b2];
			pa_+=2;
		}
		else
		{
			*pw_++=*pa_++;
		}
	}
	dst.resize(pw_-dst.data());
	dst.swap(aw_);
	return true;
}

bool IConv::gbk_to_unicode(StringBuffer<uint16_t>& aw_,const uint8_t* pa_,size_t ln_)
{
	return IConv_gbk_to_unicode(aw_,pa_,ln_);
}

bool IConv::gbk_to_unicode(StringBuffer<uint32_t>& aw_,const uint8_t* pa_,size_t ln_)
{
	return IConv_gbk_to_unicode(aw_,pa_,ln_);
}

template<typename WC>
bool IConv_utf8_to_unicode(StringBuffer<WC>& aw_,const uint8_t* pa_,size_t ln_)
{
	StringBuffer<WC> dst;
	dst.resize(ln_);
	WC *pw_=dst.data();
	const uint8_t* pa2=pa_+ln_;

	while(pa_<pa2)
	{
		uint8_t uc=*pa_;
		if(uc<0x80)
		{
			*pw_++=uc;
			pa_+=1;
		}
		else if(uc<0xC0)
		{
			*pw_++=uc;
			pa_+=1;
		}
		else if(uc<0xE0)
		{
			*pw_++=((uc&0x3F)<<6)+(pa_[1]&0x3F);
			pa_+=2;
		}
		else if(uc<0xF0)
		{
			*pw_++=((uc&0x3F)<<12)+((pa_[1]&0x3F)<<6)+(pa_[2]&0x3F);
			pa_+=3;
		}
		else
		{
			unsigned code=((uc&0x3F)<<18)+((pa_[1]&0x3F)<<12)+((pa_[2]&0x3F)<<6)+(pa_[3]&0x3F);
			*pw_++=code;
			pa_+=4;
		}
	}

	dst.resize(pw_-dst.data());
	dst.swap(aw_);
	return true;
}

bool IConv::utf8_to_unicode(StringBuffer<uint16_t>& aw_,const uint8_t* pa_,size_t ln_)
{
	return IConv_utf8_to_unicode(aw_,pa_,ln_);
}

bool IConv::utf8_to_unicode(StringBuffer<uint32_t>& aw_,const uint8_t* pa_,size_t ln_)
{
	return IConv_utf8_to_unicode(aw_,pa_,ln_);
}


template<typename WC>
bool IConv_unicode_to_utf8(StringBuffer<uint8_t>& aa_,const WC* pw_,size_t ln_)
{
	StringBuffer<uint8_t> dst;

	dst.resize(ln_*4);
	uint8_t* pa_=dst.data();
	const WC* pw2=pw_+ln_;

	for(; pw_<pw2; pw_++)
	{
		WC code=pw_[0];
		if (code < 0x80)
		{
			*pa_++=code;
		}
		else if (code < 0x800)
		{
			pa_[1] = ((code | 0x80) & 0xBF);
			code >>= 6;
			pa_[0] = (code | 0xC0);
			pa_+=2;
		}
		else if (code < 0x10000)
		{
			pa_[2] = ((code|0x80) & 0xBF);
			code >>= 6;
			pa_[1] = ((code|0x80) & 0xBF);
			code >>= 6;
			pa_[0] = (code|0xE0);
			pa_+=3;
		}
		else if (code < 0x110000)
		{
			pa_[3] = ((code | 0x80) & 0xBF);
			code >>= 6;
			pa_[2] = ((code | 0x80) & 0xBF);
			code >>= 6;
			pa_[1] = ((code | 0x80) & 0xBF);
			code >>= 6;
			pa_[0] = (code | 0xF0);
			pa_+=4;
		}
		else
		{
			System::SetLastError(String::Format("IConv_unicode_to_utf8: unkown unicode character %x",(uint32_t)code));
			return false;
		}
	}

	dst.resize(pa_-dst.data());
	dst.swap(aa_);

	return true;
}

bool IConv::unicode_to_utf8(StringBuffer<uint8_t>& aa_,const uint16_t* pw_,size_t ln_)
{
	return IConv_unicode_to_utf8(aa_,pw_,ln_);
}

bool IConv::unicode_to_utf8(StringBuffer<uint8_t>& aa_,const uint32_t* pw_,size_t ln_)
{
	return IConv_unicode_to_utf8(aa_,pw_,ln_);
}


bool IConv::utf8_to_ansi(StringBuffer<char>& aa_,const char* pa_,size_t ln_)
{
#ifdef EW_WINDOWS
	StringBuffer<wchar_t> aw;
	if(!utf8_to_unicode(aw,pa_,ln_))
	{
		return false;
	}
	if(!unicode_to_gbk(aa_,aw.data(),aw.size()))
	{
		return false;
	}
	return true;
#else
	aa_.append(pa_,ln_);
	return true;
#endif
}

bool IConv::ansi_to_utf8(StringBuffer<char>& aa_,const char* pa_,size_t ln_)
{
#ifdef EW_WINDOWS
	StringBuffer<wchar_t> aw;
	if(!gbk_to_unicode(aw,pa_,ln_)) return false;
	return unicode_to_utf8(aa_,aw.data(),aw.size());
#else
	aa_.append(pa_,ln_);
	return true;
#endif
}


bool IConv::utf8_to_gbk(StringBuffer<char>& aa_,const char* pa_,size_t ln_)
{
	StringBuffer<uint16_t> aw;
	if(!utf8_to_unicode(aw,pa_,ln_)) return false;
	return unicode_to_gbk(aa_,aw.data(),aw.size());
}

bool IConv::gbk_to_utf8(StringBuffer<char>& aa_,const char* pa_,size_t ln_)
{
	StringBuffer<uint16_t> aw;
	if(!gbk_to_unicode(aw,pa_,ln_)) return false;
	return unicode_to_utf8(aa_,aw.data(),aw.size());
}

EW_LEAVE
