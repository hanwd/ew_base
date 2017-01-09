#ifndef __H_EW_STRING_DETAIL_H__
#define __H_EW_STRING_DETAIL_H__

#include "ewa_base/config.h"
#include "ewa_base/basic/exception.h"
#include <cstring>
#include <cstdarg>


EW_ENTER

class DLLIMPEXP_EWA_BASE String;


template<typename T>
class DLLIMPEXP_EWA_BASE StringBuffer;

class StringParamCast
{
public:
	template<typename G>
	static inline G g(const G& v)
	{
		return v;
	}

	static inline const char* g(const char* v)
	{
		return v;
	}

	static inline const char* g(const std::string& v)
	{
		return v.c_str();
	}

	static const char* g(const String& v);


	static const char* g(const StringBuffer<char>& v);
};

DLLIMPEXP_EWA_BASE void* mp_alloc(size_t n);
DLLIMPEXP_EWA_BASE void mp_free(void* p);


class StringDetail
{
public:

	typedef char char_type;

	static inline char_type* str_empty()
	{
		return (char_type*)const_empty_buffer;
	}

	static inline char_type* str_alloc(size_t s)
	{
		char_type* p=(char_type*)mp_alloc(s+1);
		if(p==NULL) Exception::XBadAlloc();
		p[s]=0;return p;
	}

	static inline void str_free(const char_type* p)
	{
		if(p!=const_empty_buffer)
		{
			mp_free((void*)p);
		}
	}

	static inline char_type* str_dup(const char_type* s)
	{
		size_t n=std::char_traits<char_type>::length(s);
		return str_dup(s,n);
	}

	static inline char_type* str_dup(const char_type* s,size_t n)
	{
		if(n==0)
		{
			return str_empty();
		}
		char_type* dst=str_alloc(n);
		memcpy(dst,s,sizeof(char_type)*n);
		return dst;
	}

	static inline char_type* str_cat(const char_type* p1,const char_type* p2)
	{
		size_t n1=std::char_traits<char_type>::length(p1);
		size_t n2=std::char_traits<char_type>::length(p2);
		return str_cat(p1,n1,p2,n2);
	}

	static inline char_type* str_cat(const char_type* p1,size_t n1,const char_type* p2,size_t n2)
	{
		size_t n=n1+n2;
		char_type* dst=str_alloc(n);
		memcpy(dst,p1,sizeof(char_type)*n1);
		memcpy(dst+n1,p2,sizeof(char_type)*n2);
		return dst;
	}

	template<typename T,typename G>
	static T* str_format(T* p1,G v_)
	{
		typedef typename unsigned_integer_type<sizeof(G)>::type U;

		*p1=T(0);
		if(v_>0)
		{
			U v=v_;
			while(v>0)
			{
				*--p1='0'+(v%10);
				v=v/10;
			}
		}
		else if(v_==0)
		{
			*--p1='0';
		}
		else
		{
			U v=(~(U)v_)+1;
			while(v>0)
			{
				*--p1='0'+(v%10);
				v=v/10;
			}
			*--p1='-';
		}

		return p1;
	}
};



#define STRING_FORMAT_PTvar(x) typename T##x
#define STRING_FORMAT_PMvar(x) const T##x & p##x
#define STRING_FORMAT_PKvar(x) StringParamCast::g(p##x)

#define STRING_FORMAT_PMList0 STRING_FORMAT_PMvar(0)
#define STRING_FORMAT_PMList1 STRING_FORMAT_PMList0,STRING_FORMAT_PMvar(1)
#define STRING_FORMAT_PMList2 STRING_FORMAT_PMList1,STRING_FORMAT_PMvar(2)
#define STRING_FORMAT_PMList3 STRING_FORMAT_PMList2,STRING_FORMAT_PMvar(3)
#define STRING_FORMAT_PMList4 STRING_FORMAT_PMList3,STRING_FORMAT_PMvar(4)
#define STRING_FORMAT_PMList5 STRING_FORMAT_PMList4,STRING_FORMAT_PMvar(5)
#define STRING_FORMAT_PMList6 STRING_FORMAT_PMList5,STRING_FORMAT_PMvar(6)
#define STRING_FORMAT_PMList7 STRING_FORMAT_PMList6,STRING_FORMAT_PMvar(7)
#define STRING_FORMAT_PMList8 STRING_FORMAT_PMList7,STRING_FORMAT_PMvar(8)
#define STRING_FORMAT_PMList9 STRING_FORMAT_PMList8,STRING_FORMAT_PMvar(9)

#define STRING_FORMAT_PKList0 STRING_FORMAT_PKvar(0)
#define STRING_FORMAT_PKList1 STRING_FORMAT_PKList0,STRING_FORMAT_PKvar(1)
#define STRING_FORMAT_PKList2 STRING_FORMAT_PKList1,STRING_FORMAT_PKvar(2)
#define STRING_FORMAT_PKList3 STRING_FORMAT_PKList2,STRING_FORMAT_PKvar(3)
#define STRING_FORMAT_PKList4 STRING_FORMAT_PKList3,STRING_FORMAT_PKvar(4)
#define STRING_FORMAT_PKList5 STRING_FORMAT_PKList4,STRING_FORMAT_PKvar(5)
#define STRING_FORMAT_PKList6 STRING_FORMAT_PKList5,STRING_FORMAT_PKvar(6)
#define STRING_FORMAT_PKList7 STRING_FORMAT_PKList6,STRING_FORMAT_PKvar(7)
#define STRING_FORMAT_PKList8 STRING_FORMAT_PKList7,STRING_FORMAT_PKvar(8)
#define STRING_FORMAT_PKList9 STRING_FORMAT_PKList8,STRING_FORMAT_PKvar(9)

#define STRING_FORMAT_PTList0 STRING_FORMAT_PTvar(0)
#define STRING_FORMAT_PTList1 STRING_FORMAT_PTList0,STRING_FORMAT_PTvar(1)
#define STRING_FORMAT_PTList2 STRING_FORMAT_PTList1,STRING_FORMAT_PTvar(2)
#define STRING_FORMAT_PTList3 STRING_FORMAT_PTList2,STRING_FORMAT_PTvar(3)
#define STRING_FORMAT_PTList4 STRING_FORMAT_PTList3,STRING_FORMAT_PTvar(4)
#define STRING_FORMAT_PTList5 STRING_FORMAT_PTList4,STRING_FORMAT_PTvar(5)
#define STRING_FORMAT_PTList6 STRING_FORMAT_PTList5,STRING_FORMAT_PTvar(6)
#define STRING_FORMAT_PTList7 STRING_FORMAT_PTList6,STRING_FORMAT_PTvar(7)
#define STRING_FORMAT_PTList8 STRING_FORMAT_PTList7,STRING_FORMAT_PTvar(8)
#define STRING_FORMAT_PTList9 STRING_FORMAT_PTList8,STRING_FORMAT_PTvar(9)

#define STRING_FORMAT_PFList0(X,Y,Z) template<STRING_FORMAT_PTList0> X(STRING_FORMAT_PMList0){Y(STRING_FORMAT_PKList0,Z);}
#define STRING_FORMAT_PFList1(X,Y,Z) template<STRING_FORMAT_PTList1> X(STRING_FORMAT_PMList1){Y(STRING_FORMAT_PKList1,Z);}
#define STRING_FORMAT_PFList2(X,Y,Z) template<STRING_FORMAT_PTList2> X(STRING_FORMAT_PMList2){Y(STRING_FORMAT_PKList2,Z);}
#define STRING_FORMAT_PFList3(X,Y,Z) template<STRING_FORMAT_PTList3> X(STRING_FORMAT_PMList3){Y(STRING_FORMAT_PKList3,Z);}
#define STRING_FORMAT_PFList4(X,Y,Z) template<STRING_FORMAT_PTList4> X(STRING_FORMAT_PMList4){Y(STRING_FORMAT_PKList4,Z);}
#define STRING_FORMAT_PFList5(X,Y,Z) template<STRING_FORMAT_PTList5> X(STRING_FORMAT_PMList5){Y(STRING_FORMAT_PKList5,Z);}
#define STRING_FORMAT_PFList6(X,Y,Z) template<STRING_FORMAT_PTList6> X(STRING_FORMAT_PMList6){Y(STRING_FORMAT_PKList6,Z);}
#define STRING_FORMAT_PFList7(X,Y,Z) template<STRING_FORMAT_PTList7> X(STRING_FORMAT_PMList7){Y(STRING_FORMAT_PKList7,Z);}
#define STRING_FORMAT_PFList8(X,Y,Z) template<STRING_FORMAT_PTList8> X(STRING_FORMAT_PMList8){Y(STRING_FORMAT_PKList8,Z);}
#define STRING_FORMAT_PFList9(X,Y,Z) template<STRING_FORMAT_PTList9> X(STRING_FORMAT_PMList9){Y(STRING_FORMAT_PKList9,Z);}

#define STRING_FORMAT_FUNCTIONS_2(X,Y1,Y2,Z)  \
	STRING_FORMAT_PFList0(X,Y1,Z);\
	STRING_FORMAT_PFList1(X,Y2,Z);\
	STRING_FORMAT_PFList2(X,Y2,Z);\
	STRING_FORMAT_PFList3(X,Y2,Z);\
	STRING_FORMAT_PFList4(X,Y2,Z);\
	STRING_FORMAT_PFList5(X,Y2,Z);\
	STRING_FORMAT_PFList6(X,Y2,Z);\
	STRING_FORMAT_PFList7(X,Y2,Z);\
	STRING_FORMAT_PFList8(X,Y2,Z);\
	STRING_FORMAT_PFList9(X,Y2,Z);

#define STRING_FORMAT_FUNCTIONS(X,Y,Z)  STRING_FORMAT_FUNCTIONS_2(X,Y,Y,Z)



#define STRING_FORMATER_FORMAT_FUNCS_3(X,Y,Z)\
template<typename T0,\
	typename T1=tl::nulltype,\
	typename T2=tl::nulltype,\
	typename T3=tl::nulltype,\
	typename T4=tl::nulltype,\
	typename T5=tl::nulltype,\
	typename T6=tl::nulltype,\
	typename T7=tl::nulltype,\
	typename T8=tl::nulltype,\
	typename T9=tl::nulltype\
>\
X(\
	const T0& v0,\
	const T1& v1=T1(),\
	const T2& v2=T2(),\
	const T3& v3=T3(),\
	const T4& v4=T4(),\
	const T5& v5=T5(),\
	const T6& v6=T6(),\
	const T7& v7=T7(),\
	const T8& v8=T8(),\
	const T9& v9=T9()\
)\
{\
	Y sb(StringParamCast::g(v0));\
	StringFormater::Format(sb,v1,v2,v3,v4,v5,v6,v7,v8,v9);\
	Z;\
}

#define STRING_FORMATER_FORMAT_FUNCS_2(X,Z) STRING_FORMATER_FORMAT_FUNCS_3(X,FormatStateSb,Z)

EW_LEAVE
#endif
