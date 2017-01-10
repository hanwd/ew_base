#include "ewa_base/basic/formater.h"
#include "ewa_base/basic/string.h"
EW_ENTER


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

template class FormatStateT<StringBuffer<char> >;
template class FormatStateT<FormatBuffer >;

template<typename T0>
bool StringFormater::handle(T0& st)
{

	while(*st.p2)
	{
		if(*st.p2=='%')
		{
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
		else
		{
			st.p2++;
		}				
	}

	st.str_append(st.p1,st.p2);

	return false;
}

template bool StringFormater::handle(FormatStateSb& st);
template bool StringFormater::handle(FormatStateFb& st);

void FormatState0::init(const wchar_t* p)
{
	phold.reset(new StringBuffer<char>);
	IConv::unicode_to_utf8(*phold,p,std::char_traits<wchar_t>::length(p));
	init(phold->data());
}


EW_LEAVE
