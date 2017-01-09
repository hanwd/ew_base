#ifndef __H_EW_BASIC_FORMATER__
#define __H_EW_BASIC_FORMATER__

#include "ewa_base/config.h"
#include "ewa_base/basic/stringbuffer.h"
#include "ewa_base/basic/string.h"

EW_ENTER





class StringFormater
{
public:
	
	typedef tl::nulltype nil_type;

	class str_type_info
	{
	public:
		int type;
		int size;
	};

	static const indexer_map<String,int>& get_type_spec_map();

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

	class buffer
	{
	public:
		char stat[1024];
		char* addr;
		size_t size;

		char* get_buffer(size_t s)
		{
			if(s<=size) return addr;
			if(addr!=stat)
			{
				mp_free(addr);
			}
			addr=(char*)mp_alloc(s);
			if(!addr)
			{
				Exception::XBadAlloc();
			}
			size=s;
			return addr;
		}

		buffer()
		{
			addr=stat;
			size=1024;			
		}

		~buffer()
		{
			if(addr!=stat)
			{
				mp_free(addr);
			}
		}

	};

	class state
	{
	public:
		typedef const char* char_pointer;
		state(char_pointer p):p1(p),p2(p),kp(0),fg(0),sz(0){}
		state(const String& s):p1(s.c_str()),p2(s.c_str()),kp(0),fg(0),sz(0){}

		char_pointer p1,p2,f1,f2;

		int64_t kp;
		int fg;
		int ty;
		StringBuffer<char> sb;
		buffer ss;
		buffer fb;
		int64_t wd;
		int64_t dd;

		int sz;

		operator String()
		{
			return sb;
		}

		void append_s(const char* x)
		{
			append(x);
		}

		void append_s(const String& x)
		{
			append(x.c_str());
		}

		template<typename G>
		void append_s(const G&)
		{
			append("??");
		}

		void append(const String& s)
		{
			append(s.c_str());
		}

		void append(char_pointer x1)
		{
			char_pointer x2=x1+::strlen(x1);
			append(x1,x2);
		}

		void append(char_pointer x1,char_pointer x2)
		{
			sb.append(x1,x2);
		}

		template<typename G>
		void append_t(const G& o)
		{
			sb<<o;
		}
	};
	

	template<typename G>
	static bool DoFromat(state& st,const G& o)
	{
		if(st.f1==st.f2)
		{
			st.append_t(o);
		}
		else if(st.f2[-1]=='s')
		{
			st.append_s(o);
		}
		else if(st.f2[-1]=='n')
		{

		}
		else if(st.f2-st.f1<1000)
		{
			char* fb=st.fb.get_buffer(st.f2-st.f1+2);
			fb[0]='%';
			memcpy(fb+1,st.f1,st.f2-st.f1);
			fb[st.f2-st.f1+1]=0;

			unsigned wd=unsigned(st.wd)+unsigned(st.dd);
			char* ss=st.ss.get_buffer(wd+512);
			
			int nd=sprintf(ss,fb,o);
			if(nd>=0)
			{
				st.append(ss,ss+nd);
			}
			else
			{
				return false;
			}
			

		}

		return true;
	
	}

	static bool DoFromat(state&,const nil_type&)
	{
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
	static String Format(const T0& v0,
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
		const indexer_map<String,int>& hmap(get_type_spec_map());

		state st(v0);

		while(*st.p2)
		{
			if(*st.p2=='%')
			{
				if(st.p2[1]=='%')
				{
					st.append(st.p1,++st.p2);
					st.p1=++st.p2;
					continue;
				}

				st.append(st.p1,st.p2);
				st.p1=st.p2++;
					
				if(*st.p2=='{')
				{
					ScannerHelper<const char*>::read_uint(++st.p2,st.kp);
					if(*st.p2!=',')
					{
						st.fg=0;
					}
					else
					{
						st.fg=-1;	
						++st.p2;		
					}
				}
				else
				{
					st.fg=1;
					st.kp++;
				}

				if(st.fg!=0)
				{
					st.f1=st.p2;
					st.wd=0;
					st.dd=0;

					if(*st.p2=='+'||*st.p2=='-') st.p2++;
					if(*st.p2=='0') st.p2++;
					if(*st.p2=='+'||*st.p2=='-') st.p2++;

					ScannerHelper<const char*>::read_uint(st.p2,st.wd);
					if(*st.p2=='.')
					{
						st.p2++;
						ScannerHelper<const char*>::read_uint(st.p2,st.dd);
					}

					if(st.p2[0]=='h'||st.p2[0]=='l')
					{
						if(st.p2[1]==st.p2[0])
						{
							//st.ty=hmap[String(st.p2,st.p2+3)];
							st.p2+=3;
						}
						else
						{
							//st.ty=hmap[String(st.p2,st.p2+2)];
							st.p2+=2;			
						}
					}
					else if(st.p2[0]=='j'||st.p2[0]=='z'||st.p2[0]=='t'||st.p2[0]=='L')
					{
						//st.ty=hmap[String(st.p2,st.p2+2)];
						st.p2+=2;	
					}
					else
					{
						//st.ty=hmap[String(st.p2,st.p2+1)];
						st.p2+=1;		
					}

					st.f2=st.p2;

					if(st.ty==0)
					{
						continue;
					}				
				}
				else
				{
					st.f1=st.f2;
				}
					
				if(st.fg<=0)
				{
					if(*st.p2++!='}')
					{
						System::LogTrace("invalid format");
						continue;
					}
				}

				bool flag=false;
				switch(st.kp)
				{
				case 1: flag=DoFromat(st,v1);break;
				case 2: flag=DoFromat(st,v2);break;
				case 3: flag=DoFromat(st,v3);break;
				case 4: flag=DoFromat(st,v4);break;
				case 5: flag=DoFromat(st,v5);break;
				case 6: flag=DoFromat(st,v6);break;
				case 7: flag=DoFromat(st,v7);break;
				case 8: flag=DoFromat(st,v8);break;
				case 9: flag=DoFromat(st,v9);break;
				}

				if(!flag) continue;

				st.p1=st.p2;		

			}
			else
			{
				st.p2++;
			}					
		}

		st.append(st.p1,st.p2);
		return st;
	}
};


EW_LEAVE

#endif
