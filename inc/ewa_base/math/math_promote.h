
#ifndef __H_EW_MATH_MATH_PROMOTE__
#define __H_EW_MATH_MATH_PROMOTE__


#include "ewa_base/config.h"


EW_ENTER


namespace detail
{

	template<typename T>
	class scalar_flag
	{
	public:
		static const int value=-1;
	};

	template<>
	class scalar_flag<int32_t>
	{
	public:
		static const int value=0;
	};

	template<>
	class scalar_flag<int64_t>
	{
	public:
		static const int value=1;
	};

	template<>
	class scalar_flag<float>
	{
	public:
		static const int value=2;
	};

	template<>
	class scalar_flag<double>
	{
	public:
		static const int value=3;
	};

	template<int N>
	class scalar_type : public tl::value_type<false>
	{
	public:

	};

	template<>
	class scalar_type<0> : public tl::value_type<true>
	{
	public:
		typedef int32_t type;
	};

	template<>
	class scalar_type<1> : public tl::value_type<true>
	{
	public:
		typedef int64_t type;
	};

	template<>
	class scalar_type<2> : public tl::value_type<true>
	{
	public:
		typedef float type;
	};

	template<>
	class scalar_type<3> : public tl::value_type<true>
	{
	public:
		typedef double type;
	};
}

template<typename X,typename Y>
class scr_promote : public detail::scalar_type<detail::scalar_flag<X>::value|detail::scalar_flag<Y>::value>
{
public:

};


template<typename X,typename Y,template<typename,typename> class B,template<typename> class H,int N=-1>
struct opx_helper_promote;

template<typename X,typename Y,template<typename,typename> class B,template<typename> class H>
struct opx_helper_promote<X,Y,B,H,0> : public tl::value_type<false>{};

template<typename X,typename Y,template<typename,typename> class B,template<typename> class H>
struct opx_helper_promote<X,Y,B,H,1> : public tl::value_type<true>
{
	typedef typename B<X,Y>::type scalar;
	typedef typename H<scalar>::promoted promoted;
	typedef scalar type;
};

template<typename X,typename Y,template<typename,typename> class B,template<typename> class H>
struct opx_helper_promote<X,Y,B,H,2> : public tl::value_type<true>
{
	typedef typename B<typename H<X>::type,typename H<Y>::type>::type scalar;
	typedef typename H<scalar>::promoted promoted;
	typedef promoted type;
};
template<typename X,typename Y,template<typename,typename> class B,template<typename> class H>
struct opx_helper_promote<X,Y,B,H,-1> : 
	public opx_helper_promote<X,Y,B,H,B<typename H<X>::type,typename H<Y>::type>::value?(H<X>::value||H<Y>::value?2:1):0>
{

};



EW_LEAVE

#endif
