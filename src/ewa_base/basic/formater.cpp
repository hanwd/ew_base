#include "ewa_base/basic/formater.h"
#include "ewa_base/basic/string.h"
EW_ENTER

FormatBuffer& FormatBuffer::operator<<(const String& s)
{
	append(s.c_str(),s.size());
	return *this;
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

//
//template<typename T>
//static void add_int_type(indexer_map<String,int>& hmap,const String& pre,int sign=0)
//{
//	int itypes[9]={0,1,2,0,3,0,0,0,4};
//	EW_ASSERT(sizeof(T)<=8);
//	EW_ASSERT(itypes[sizeof(T)]!=0);
//	int f1=0,f2=StringFormater::TYPE_UNSIGNED;
//	if(sign==+1) f1=f2=StringFormater::TYPE_UNSIGNED;
//	if(sign==-1) f1=f2=0;
//	hmap[pre+"i"]=hmap[pre+"d"]=itypes[sizeof(T)]|f1;
//	hmap[pre+"u"]=hmap[pre+"x"]=hmap[pre+"X"]=itypes[sizeof(T)]|f2;
//}
//	
//const indexer_map<String,int>& StringFormater::get_type_spec_map()
//{
//
//	static indexer_map<String,int> hmap;
//	if(hmap.empty())
//	{
//		add_int_type<int>(hmap,"");
//		add_int_type<signed char>(hmap,"hh");
//		add_int_type<short int>(hmap,"h");
//		add_int_type<long int>(hmap,"l");
//		add_int_type<long long int>(hmap,"ll");
//		add_int_type<intmax_t>(hmap,"j");
//		add_int_type<size_t>(hmap,"z",+1);
//		add_int_type<ptrdiff_t>(hmap,"z",-1);
//
//		hmap["f"]=hmap["F"]=hmap["e"]=hmap["E"]=hmap["g"]=hmap["a"]=hmap["A"]=TYPE_FLOAT64;
//		hmap["Lf"]=hmap["LF"]=hmap["Le"]=hmap["LE"]=hmap["Lg"]=hmap["La"]=hmap["LA"]=TYPE_FLOAT64;
//		hmap["lf"]=hmap["lF"]=hmap["le"]=hmap["lE"]=hmap["lg"]=hmap["la"]=hmap["lA"]=TYPE_FLOAT64;
//		hmap["p"]=TYPE_POINTER;
//		hmap["n"]=TYPE_NULL;
//		hmap["c"]=TYPE_INT32;
//		hmap["s"]=TYPE_STRING;
//		hmap["ls"]=TYPE_WSTRING;	
//	}
//
//	return hmap;
//}




EW_LEAVE
