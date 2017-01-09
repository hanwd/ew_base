#include "ewa_base/basic/formater.h"

EW_ENTER


template<typename T>
static void add_int_type(indexer_map<String,int>& hmap,const String& pre,int sign=0)
{
	int itypes[9]={0,1,2,0,3,0,0,0,4};
	EW_ASSERT(sizeof(T)<=8);
	EW_ASSERT(itypes[sizeof(T)]!=0);
	int f1=0,f2=StringFormater::TYPE_UNSIGNED;
	if(sign==+1) f1=f2=StringFormater::TYPE_UNSIGNED;
	if(sign==-1) f1=f2=0;
	hmap[pre+"i"]=hmap[pre+"d"]=itypes[sizeof(T)]|f1;
	hmap[pre+"u"]=hmap[pre+"x"]=hmap[pre+"X"]=itypes[sizeof(T)]|f2;
}
	
const indexer_map<String,int>& StringFormater::get_type_spec_map()
{

	static indexer_map<String,int> hmap;
	if(hmap.empty())
	{
		add_int_type<int>(hmap,"");
		add_int_type<signed char>(hmap,"hh");
		add_int_type<short int>(hmap,"h");
		add_int_type<long int>(hmap,"l");
		add_int_type<long long int>(hmap,"ll");
		add_int_type<intmax_t>(hmap,"j");
		add_int_type<size_t>(hmap,"z",+1);
		add_int_type<ptrdiff_t>(hmap,"z",-1);

		hmap["f"]=hmap["F"]=hmap["e"]=hmap["E"]=hmap["g"]=hmap["a"]=hmap["A"]=TYPE_FLOAT64;
		hmap["Lf"]=hmap["LF"]=hmap["Le"]=hmap["LE"]=hmap["Lg"]=hmap["La"]=hmap["LA"]=TYPE_FLOAT64;
		hmap["lf"]=hmap["lF"]=hmap["le"]=hmap["lE"]=hmap["lg"]=hmap["la"]=hmap["lA"]=TYPE_FLOAT64;
		hmap["p"]=TYPE_POINTER;
		hmap["n"]=TYPE_NULL;
		hmap["c"]=TYPE_INT32;
		hmap["s"]=TYPE_STRING;
		hmap["ls"]=TYPE_WSTRING;	
	}

	return hmap;
}


EW_LEAVE
