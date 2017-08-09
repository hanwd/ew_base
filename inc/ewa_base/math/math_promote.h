
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


EW_LEAVE

#endif
