#ifndef __H_EW_BASIC_FORMATER__
#define __H_EW_BASIC_FORMATER__

#include "ewa_base/config.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/scanner_helper.h"


EW_ENTER


class DLLIMPEXP_EWA_BASE FormatBuffer : public FormatHelper<FormatBuffer,FormatPolicy2>
{
public:

	void clear(){m_size=0;}

	char* c_str(){p_base[m_size]=0;return p_base;}

	char* data(){return p_base;}

	size_t size(){return m_size;}

	size_t capacity(){return m_capacity;}

	void append(const char* p,size_t n)
	{
		if(n+m_size>=m_capacity)
		{			
			reserve(n+m_size+512);
		}

		memcpy(p_base+m_size,p,n);
		m_size+=n;
	}

	void enlarge_size_by(size_t n)
	{
		m_size+=n;
	}


	FormatBuffer()
	{
		p_base=internal_data;
		m_capacity=sizeof(internal_data);
		m_size=0;
	}

	~FormatBuffer()
	{
		if(p_base!=internal_data)
		{
			mp_free(p_base);
		}
	}

	void reserve(size_t n)
	{
		if(n<m_capacity) return;

		size_t sz2=(n+1024*8)&~(1024*4-1);
		char* ptmp=(char*)mp_alloc(sz2);
		if(!ptmp) Exception::XBadAlloc();
		memcpy(ptmp,p_base,m_size);
		if(p_base!=internal_data)
		{
			mp_free(p_base);
		}
		p_base=ptmp;
		m_capacity=sz2;
	}

private:

	char* p_base;
	size_t m_size;
	size_t m_capacity;
	char internal_data[1024*4];
};


class DLLIMPEXP_EWA_BASE FormatState0
{
public:
	typedef const char* char_pointer;

	FormatState0(){}

	char_pointer p1,p2;
	char_pointer f1,f2;

	int64_t n_vpos;
	int64_t n_vpos_old;
	int64_t n_fmt_width[2];
	int n_fmt_flag;
	int n_fmt_type;

	bool b_fmt_ok;

	void init(char_pointer p){p1=p2=p;n_vpos=0;}

	void init(const wchar_t* p);


	std::auto_ptr<StringBuffer<char> > phold;

};


class DLLIMPEXP_EWA_BASE FormatState1 : public FormatState0
{
public:

	FormatBuffer a_fmt_buffer;

	void fmt_enter()
	{
		b_fmt_ok=true;
		a_fmt_buffer.clear();
		n_fmt_width[0]=n_fmt_width[1]=0;
		fmt_append("%",1);
	}

	bool fmt_test()
	{
		if(b_fmt_ok)
		{
			f1=a_fmt_buffer.c_str();
			f2=f1+a_fmt_buffer.size();
			if(a_fmt_buffer.size()==1)
			{
				f1=f2=NULL;
			}
		}
		return b_fmt_ok;
	}

	bool fmt_leave()
	{
		if(!b_fmt_ok)
		{
			n_vpos=n_vpos_old;
		}
		return b_fmt_ok;
	}

	void fmt_append(const char* x1,const char* x2)
	{
		a_fmt_buffer.append(x1,x2-x1);
	}

	void fmt_append(const char* p,size_t n)
	{
		a_fmt_buffer.append(p,n);
	}

	template<typename G>
	void fmt_width(const G&,int i)
	{
		b_fmt_ok=false;
	}

	void fmt_width(int d,int i)
	{
		n_fmt_width[i]=d;
		a_fmt_buffer<<d;
	}
	
};

template<typename S>
class DLLIMPEXP_EWA_BASE FormatStateT : public FormatState1
{
public:

	FormatStateT(const char* p){init(p);}
	FormatStateT(const wchar_t* p){init(p);}

	const char* c_str(){return sb.c_str();}

	S sb;

	void str_append_s(const char* x){str_append(x);}
	void str_append_s(const wchar_t* x);

	template<typename T>
	void str_append_s(const StringBuffer<T>& x){str_append(x.c_str());}

	void str_append_s(const String& x);

	template<typename G>
	void str_append_s(const G&)
	{
		str_append("??");
	}

	void str_append(const String& s);

	void str_append(char_pointer x1)
	{
		sb.append(x1,::strlen(x1));
	}

	void str_append(char_pointer x1,char_pointer x2)
	{
		sb.append(x1,x2-x1);
	}

	template<typename G>
	void str_append_t(const G& o)
	{
		sb<<o;
	}

	template<typename G>
	void str_format_t(const G& o)
	{

		if(f1==f2)
		{
			str_append_t(o);
		}
		else if(f1[1]=='s')
		{
			str_append_s(o);
		}
		else if(f2[-1]=='n')
		{

		}
		else
		{
			size_t n=std::max(size_t(n_fmt_width[0]),size_t(n_fmt_width[1]));
			S::Policy::format(sb,std::max((size_t)FormatPolicy::width(o),n),f1,FormatPolicy::cast(o));
		}	
	}

	void str_format_t(const tl::nulltype&)
	{
		b_fmt_ok=false;
	}

};
	

typedef FormatStateT<StringBuffer<char> > FormatStateSb;
typedef FormatStateT<FormatBuffer > FormatStateFb;

template<unsigned N>
class lkt_format_characters
{
public:
	static const int value0=(N=='d'||N=='i'||N=='u'||N=='o'||N=='x'||N=='X')?1:0;
	static const int value1=(N=='f'||N=='F'||N=='e'||N=='E'||N=='g'||N=='G'||N=='a'||N=='A')?2:0;
	static const int value2=(N=='c'||N=='s'||N=='p'||N=='n')?3:0;
	static const int value=value0|value1|value2;

};

class DLLIMPEXP_EWA_BASE StringFormater
{
public:
	
	typedef tl::nulltype nil_type;

	class str_type_info
	{
	public:
		int type;
		int size;
	};

	enum
	{
		TYPE_NULL,
		TYPE_INT8,
		TYPE_INT16,
		TYPE_INT32,
		TYPE_INT64,
		TYPE_POINTER,
		TYPE_FLOAT32,
		TYPE_FLOAT64,
		TYPE_STRING,
		TYPE_WSTRING,
		TYPE_UNSIGNED=1<<6
	};


	template<typename T0>
	static bool handle(T0& st)
	{
		while(*st.p2)
		{
			if(*st.p2!='%')
			{
				st.p2++;
				continue;
			}
	
			if(st.p2[1]=='%')
			{
				st.str_append(st.p1,++st.p2);
				st.p1=++st.p2;
				continue;
			}

			st.str_append(st.p1,st.p2);
			st.p1=st.p2++;

			st.n_vpos_old=st.n_vpos;

			if(*st.p2=='{')
			{
				ScannerHelper<const char*>::read_uint(++st.p2,st.n_vpos);
				if(*st.p2!=',')
				{
					st.n_fmt_flag=0;
				}
				else
				{
					st.n_fmt_flag=-1;	
					++st.p2;		
				}
			}
			else
			{
				st.n_fmt_flag=1;
				st.n_vpos++;
			}

			return true;
			
		}

		st.str_append(st.p1,st.p2);

		return false;
	}
	template<typename T0,
		typename T1=nil_type,
		typename T2=nil_type,
		typename T3=nil_type,
		typename T4=nil_type,
		typename T5=nil_type,
		typename T6=nil_type,
		typename T7=nil_type,
		typename T8=nil_type,
		typename T9=nil_type
	>
	static void Format(T0& st,
	const T1& v1=T1(),
	const T2& v2=T2(),
	const T3& v3=T3(),
	const T4& v4=T4(),
	const T5& v5=T5(),
	const T6& v6=T6(),
	const T7& v7=T7(),
	const T8& v8=T8(),
	const T9& v9=T9()
	
	)
	{

		while(handle(st))
		{
			st.fmt_enter();

			if(st.n_fmt_flag!=0)
			{

				st.f1=st.p2;

				while(*st.p2=='+'||*st.p2=='-'||*st.p2=='0'||*st.p2==' '||*st.p2=='#') st.p2++;

				for(int i=0;i<2;i++)
				{
					if(*st.p2!='*')
					{
						ScannerHelper<const char*>::read_uint(st.p2,st.n_fmt_width[i]);
					}
					else
					{
						st.fmt_append(st.f1,st.p2);
						st.f1=++st.p2;

						switch(st.n_vpos++)
						{
						case 1:st.fmt_width(v1,i);break;
						case 2:st.fmt_width(v2,i);break;
						case 3:st.fmt_width(v3,i);break;
						case 4:st.fmt_width(v4,i);break;
						case 5:st.fmt_width(v5,i);break;
						case 6:st.fmt_width(v6,i);break;
						case 7:st.fmt_width(v7,i);break;
						case 8:st.fmt_width(v8,i);break;
						case 9:st.fmt_width(v9,i);break;
						default:st.fmt_width(tl::nulltype(),i);break;
						}
					}

					if(*st.p2=='.')
					{
						st.p2++;
					}
					else
					{
						break;
					}
				}
		
				if(st.p2[0]=='h'||st.p2[0]=='l')
				{
					if(st.p2[1]==st.p2[0])
					{
						st.p2+=3;
					}
					else
					{
						st.p2+=2;			
					}	
				}
				else if(st.p2[0]=='j'||st.p2[0]=='z'||st.p2[0]=='t'||st.p2[0]=='L')
				{
					st.p2+=2;	
				}
				else
				{
					st.p2+=1;		
				}

				if(lookup_table<lkt_format_characters>::test(st.p2[-1])==0)
				{
					continue;
				}

				st.fmt_append(st.f1,st.p2);		
			}

					
			if(st.n_fmt_flag<=0)
			{
				if(*st.p2++!='}')
				{
					st.b_fmt_ok=false;					
					continue;
				}
			}

			if(st.fmt_test()) switch(st.n_vpos)
			{
			case 1: st.str_format_t(v1);break;
			case 2: st.str_format_t(v2);break;
			case 3: st.str_format_t(v3);break;
			case 4: st.str_format_t(v4);break;
			case 5: st.str_format_t(v5);break;
			case 6: st.str_format_t(v6);break;
			case 7: st.str_format_t(v7);break;
			case 8: st.str_format_t(v8);break;
			case 9: st.str_format_t(v9);break;
			}

			if(!st.fmt_leave())
			{
				continue;
			}


			st.p1=st.p2;
					
		}

	}
};


EW_LEAVE

#endif
