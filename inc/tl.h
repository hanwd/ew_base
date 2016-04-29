#ifndef __H_EW6_TEMPLATE_LIBRARY__
#define __H_EW6_TEMPLATE_LIBRARY__



#ifdef _MSC_VER
// Disable the following warnings for Visual Studio

// this is to disable the "'this' : used in base member initializer list"
// warning you get from some of the GUI objects since all the objects
// require that their parent class be passed into their constructor.
// In this case though it is totally safe so it is ok to disable this warning.
#pragma warning(disable : 4355)

// This is a warning you get sometimes when Visual Studio performs a Koenig Lookup.
// This is a bug in visual studio.  It is a totally legitimate thing to
// expect from a compiler.
#pragma warning(disable : 4675)

// This is a warning you get from visual studio 2005 about things in the standard C++
// library being "deprecated."  I checked the C++ standard and it doesn't say jack
// about any of them (I checked the searchable PDF).   So this warning is total Bunk.
#pragma warning(disable : 4996)

// This is a warning you get from visual studio 2003:
//	warning C4345: behavior change: an object of POD type constructed with an initializer
//	of the form () will be default-initialized.
// I love it when this compiler gives warnings about bugs in previous versions of itself.
#pragma warning(disable : 4345)


// Disable warnings about conversion from size_t to unsigned long and long.
#pragma warning(disable : 4267)

// Disable warnings about conversion from double to float
#pragma warning(disable : 4244)
#pragma warning(disable : 4305)

#endif

namespace tl
{


	template<bool f>
	class value_type
	{
	public:
		static const bool value = f;
	};

	template<int N>
	class int2type
	{
	public:
		static const int value=N;
	};

	namespace detail
	{
		template<typename B>
		class test_empty : public B
		{
		public:
			int val;
		};

		template<typename B,int N>
		class test_empty_n : public value_type<N==0>{};

		template<typename B>
		class test_empty_n<B,1> : public value_type<sizeof(test_empty<B>)==sizeof(int)> {};

		template<>
		class test_empty_n<unsigned char,1> : public value_type<false> {};

		template<>
		class test_empty_n<signed char,1> : public value_type<false> {};

		template<>
		class test_empty_n<char,1> : public value_type<false> {};


	}

	template <typename T>  class is_empty_type : public detail::test_empty_n<T,sizeof(T)>{};

	namespace detail
	{
		template<typename T,bool B>
		class derive_if_empty_base;

		template<typename T> class derive_if_empty_base<T,false> {};

		template<typename T>
		class derive_if_empty_base<T,true> : public T {};
	}

	template<typename T>
	class derive_if_empty : public detail::derive_if_empty_base<T,is_empty_type<T>::value> {};

	template<bool B,typename T1,typename T2>
	class select_base : public T1 {};

	template<typename T1,typename T2>
	class select_base<false,T1,T2> : public T2 {};


// ----------------------------------------------------------------------------------------

	/*!A is_same_type

		This is a template where is_same_type<T,U>::value == true when T and U are the
		same type and false otherwise.
	!*/

	template <typename T,typename U> class is_same_type : public value_type<false> {};
	template <typename T>  class is_same_type<T,T> : public value_type<true> {};

	template<bool B,typename T1,typename T2>
	class meta_if;

	template<typename T1,typename T2>
	class meta_if<true,T1,T2>
	{
	public:
		typedef T1 type;
	};

	template<typename T1,typename T2>
	class meta_if<false,T1,T2>
	{
	public:
		typedef T2 type;
	};


	class emptytype {};
	class nulltype {};

	namespace detail
	{

		template<typename T1,typename T2>
		struct typelist_pair
		{
			typedef T1 type1;
			typedef T2 type2;
		};


		template<typename TL, typename T,unsigned int index>
		struct typelist_rep
		{
			typedef typelist_pair<typename TL::type1,typename typelist_rep<typename TL::type2,T,index-1>::type > type;
		};

		template<typename TL,typename T>
		struct typelist_rep<TL,T,0>
		{
			typedef typelist_pair<T,typename TL::type2> type;
		};


		template <typename TList, int index> struct typelist_at;

		template <int N>
		struct typelist_at< nulltype, N>
		{
			typedef nulltype type;
		};

		template <typename Head, typename Tail>
		struct typelist_at< typelist_pair<Head, Tail>,-1>
		{
			typedef nulltype type;
		};

		template <typename Head, typename Tail>
		struct typelist_at< typelist_pair<Head, Tail>, 0>
		{
			typedef Head type;
		};

		template <typename Head, typename Tail, int i>
		struct typelist_at<typelist_pair<Head, Tail>, i>
		{
			typedef typename typelist_at<Tail, i-1>::type type;
		};


		template <typename TList, typename T> struct typelist_id;

		template <typename T>
		struct typelist_id<nulltype, T>
		{
			static const int value = -1 ;
		};

		template <typename T, typename Tail>
		struct typelist_id<typelist_pair<T, Tail>, T>
		{
			static const int value = 0 ;
		};

		template <typename Head, typename Tail, typename T>
		struct typelist_id<typelist_pair<Head, Tail>, T>
		{
		private:
			static const int temp = typelist_id<Tail, T>::value;
		public:
			static const int value = temp == -1 ? -1 : 1 + temp ;
		};


		template <typename TList> struct typelist_length;

		template <>
		struct typelist_length< nulltype >
		{
			static const int value = 0 ;
		};

		template <>
		struct typelist_length< typelist_pair<nulltype, nulltype> >
		{
			static const int value = 0 ;
		};

		template <typename T, typename U>
		struct typelist_length< typelist_pair<T, U> >
		{
			static const int value = 1 + typelist_length<U>::value ;
		};


		template<typename TL1,typename TL2>
		struct typelist_cat
		{
			typedef typelist_pair< typename TL1::type1,
					typename typelist_cat
					<
					typename TL1::type2,TL2>::type
					> type;
		};

		template<typename TL2>
		struct typelist_cat< typelist_pair<nulltype,nulltype>, TL2>
		{
			typedef TL2 type;
		};

		template<typename TL2>
		struct typelist_cat< nulltype, TL2>
		{
			typedef TL2 type;
		};


		template<typename TL1,template<typename> class TL2>
		struct typelist_map
		{
			typedef typelist_pair< TL2<typename TL1::type1> ,
					typename typelist_map
					<
					typename TL1::type2,TL2>::type
					> type;
		};

		template<template<typename> class TL2>
		struct typelist_map< typelist_pair<nulltype,nulltype>, TL2>
		{
			typedef typelist_pair<nulltype,nulltype> type;
		};

		template<template<typename> class TL2>
		struct typelist_map< nulltype, TL2>
		{
			typedef nulltype type;
		};


	}


	template<typename HELPER>
	class typelist : public HELPER
	{
	public:
		typedef HELPER helper;

		static const int length=detail::typelist_length<helper>::value;

		template<typename T>
		struct id
		{
			static const int value=detail::typelist_id<helper,T>::value;
		};

		template<unsigned N>
		struct at
		{
			typedef typename detail::typelist_at<helper,N>::type type;
		};

		template<typename T>
		struct add
		{
			typedef typelist<typename detail::typelist_rep<helper,T,length+1>::type> type;
		};

		template<typename T>
		struct ins
		{
			typedef typelist<typename detail::typelist_rep<helper,T,0>::type> type;
		};

		template<unsigned N,typename T>
		struct rep
		{
			typedef typelist<typename detail::typelist_rep<helper,T,N>::type> type;
		};

		template<typename TL>
		struct cat
		{
			typedef typelist<typename detail::typelist_cat<helper,typename TL::helper>::type > type;
		};

		template<template<typename> class DT>
		struct map
		{
			typedef typelist<typename detail::typelist_map<helper,DT>::type > type;
		};

	};


	template<
	typename T1=nulltype,
			 typename T2=nulltype,
			 typename T3=nulltype,
			 typename T4=nulltype,
			 typename T5=nulltype,
			 typename T6=nulltype,
			 typename T7=nulltype,
			 typename T8=nulltype,
			 typename T9=nulltype,
			 typename T10=nulltype,
			 typename T11=nulltype,
			 typename T12=nulltype,
			 typename T13=nulltype,
			 typename T14=nulltype,
			 typename T15=nulltype,
			 typename T16=nulltype,
			 typename T17=nulltype,
			 typename T18=nulltype,
			 typename T19=nulltype,
			 typename T20=nulltype
			 >
	struct mk_typelist
	{
		typedef detail::typelist_pair<T1,typename mk_typelist<T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16,T17,T18,T19,T20>::helper> helper;
		typedef typelist<helper> type;
	};

	template<typename T1>
	struct mk_typelist<T1>
	{
		typedef detail::typelist_pair<T1,nulltype> helper;
		typedef typelist<helper> type;
	};

	template<typename T>
	class tuple_param
	{
	public:
		typedef T type;
		typedef const T& const_reference;
		typedef T& reference;
	};

	template<typename T>
	class tuple_param<const T>
	{
	public:
		typedef T type;
		typedef const T& const_reference;
		typedef const T& reference;
	};

	template<typename T>
	class tuple_param<T&>
	{
	public:
		typedef T type;
		typedef T& const_reference;
		typedef T& reference;
	};

	template<typename T>
	class tuple_param<const T&>
	{
	public:
		typedef T type;
		typedef const T& const_reference;
		typedef const T& reference;
	};


	template<bool E,typename X0,typename BS>
	class tuple_empty_helper;

	template<typename X0,typename BS>
	class tuple_empty_helper<true,X0,BS> : public BS
	{
	public:

		typedef BS basetype;
		typedef X0 type0;
		typedef typename BS::type0 type1;
		typedef typename BS::type1 type2;
		typedef typename BS::type2 type3;
		typedef typename BS::type3 type4;
		typedef typename BS::type4 type5;
		typedef typename BS::type5 type6;
		typedef typename BS::type6 type7;
		typedef typename BS::type7 type8;
		typedef typename BS::type8 type9;


		tuple_empty_helper(){}

		tuple_empty_helper(
			typename tuple_param<type0>::const_reference x0,
			typename tuple_param<type1>::const_reference x1=typename tuple_param<type1>::type(),
			typename tuple_param<type2>::const_reference x2=typename tuple_param<type2>::type(),
			typename tuple_param<type3>::const_reference x3=typename tuple_param<type3>::type(),
			typename tuple_param<type4>::const_reference x4=typename tuple_param<type4>::type(),
			typename tuple_param<type5>::const_reference x5=typename tuple_param<type5>::type(),
			typename tuple_param<type6>::const_reference x6=typename tuple_param<type6>::type(),
			typename tuple_param<type7>::const_reference x7=typename tuple_param<type7>::type(),
			typename tuple_param<type8>::const_reference x8=typename tuple_param<type8>::type(),
			typename tuple_param<type9>::const_reference x9=typename tuple_param<type9>::type()

			)
			:basetype(x1,x2,x3,x4,x5,x6,x7,x8,x9)
		{
			(void)&x0;
		}

		inline operator typename tuple_param<type0>::reference(){return *(typename tuple_param<type0>::type*)this;}
	};

	template<typename X0,typename BS>
	class tuple_empty_helper<false,X0,BS> : public BS
	{
	public:
		typedef BS basetype;
		typedef X0 type0;
		typedef typename BS::type0 type1;
		typedef typename BS::type1 type2;
		typedef typename BS::type2 type3;
		typedef typename BS::type3 type4;
		typedef typename BS::type4 type5;
		typedef typename BS::type5 type6;
		typedef typename BS::type6 type7;
		typedef typename BS::type7 type8;
		typedef typename BS::type8 type9;

		X0 val;

		tuple_empty_helper(){}

		tuple_empty_helper(
			typename tuple_param<type0>::const_reference x0,
			typename tuple_param<type1>::const_reference x1=typename tuple_param<type1>::type(),
			typename tuple_param<type2>::const_reference x2=typename tuple_param<type2>::type(),
			typename tuple_param<type3>::const_reference x3=typename tuple_param<type3>::type(),
			typename tuple_param<type4>::const_reference x4=typename tuple_param<type4>::type(),
			typename tuple_param<type5>::const_reference x5=typename tuple_param<type5>::type(),
			typename tuple_param<type6>::const_reference x6=typename tuple_param<type6>::type(),
			typename tuple_param<type7>::const_reference x7=typename tuple_param<type7>::type(),
			typename tuple_param<type8>::const_reference x8=typename tuple_param<type8>::type(),
			typename tuple_param<type9>::const_reference x9=typename tuple_param<type9>::type()

			)
			:basetype(x1,x2,x3,x4,x5,x6,x7,x8,x9),val(x0)
		{

		}

		inline operator typename tuple_param<type0>::reference(){return val;}
	};


	template<
		typename X0=nulltype,typename X1=nulltype,typename X2=nulltype,typename X3=nulltype,typename X4=nulltype,
		typename X5=nulltype,typename X6=nulltype,typename X7=nulltype,typename X8=nulltype,typename X9=nulltype
	>
	class tuple : public tuple_empty_helper<is_empty_type<X0>::value,X0,tuple<X1,X2,X3,X4,X5,X6,X7,X8,nulltype> >
	{
	public:

		typedef X0 type0;
		typedef X1 type1;
		typedef X2 type2;
		typedef X3 type3;
		typedef X4 type4;
		typedef X5 type5;
		typedef X6 type6;
		typedef X7 type7;
		typedef X8 type8;
		typedef X9 type9;

		typedef tuple_empty_helper<is_empty_type<X0>::value,X0,tuple<X1,X2,X3,X4,X5,X6,X7,X8,nulltype> > basetype;
		typedef tuple<X1,X2,X3,X4,X5,X6,X7,X8,nulltype> tupxtype;


		static const int length=1+basetype::length;

		tuple(){}

		tuple(
			typename tuple_param<type0>::const_reference x0,
			typename tuple_param<type1>::const_reference x1=typename tuple_param<type1>::type(),
			typename tuple_param<type2>::const_reference x2=typename tuple_param<type2>::type(),
			typename tuple_param<type3>::const_reference x3=typename tuple_param<type3>::type(),
			typename tuple_param<type4>::const_reference x4=typename tuple_param<type4>::type(),
			typename tuple_param<type5>::const_reference x5=typename tuple_param<type5>::type(),
			typename tuple_param<type6>::const_reference x6=typename tuple_param<type6>::type(),
			typename tuple_param<type7>::const_reference x7=typename tuple_param<type7>::type(),
			typename tuple_param<type8>::const_reference x8=typename tuple_param<type8>::type(),
			typename tuple_param<type9>::const_reference x9=typename tuple_param<type9>::type()

			)
			:basetype(x0,x1,x2,x3,x4,x5,x6,x7,x8,x9)
		{

		}

	};

	template<>
	class tuple<>
	{
	public:

		typedef tuple basetype;
		typedef tuple tupxtype;

		typedef nulltype type0;
		typedef nulltype type1;
		typedef nulltype type2;
		typedef nulltype type3;
		typedef nulltype type4;
		typedef nulltype type5;
		typedef nulltype type6;
		typedef nulltype type7;
		typedef nulltype type8;
		typedef nulltype type9;


		static const int length=0;

		tuple(){}

		tuple(
			tuple_param<type0>::const_reference x0,
			const type1& x1=type1(),
			const type2& x2=type2(),
			const type3& x3=type3(),
			const type4& x4=type4(),
			const type5& x5=type5(),
			const type6& x6=type6(),
			const type7& x7=type7(),
			const type8& x8=type8(),
			const type9& x9=type9()

			)
		{
			(void)&x0;
			(void)&x1;
			(void)&x2;
			(void)&x3;
			(void)&x4;
			(void)&x5;
			(void)&x6;
			(void)&x7;
			(void)&x8;
			(void)&x9;
		}

		inline operator type0&() {return *(nulltype*)this;}
	};

	template<typename T,int N>
	class tuple_at
	{
	public:
		typedef tuple_at<typename T::tupxtype,N-1> tupxbase;
		typedef typename tupxbase::type type;
		static inline typename tuple_param<type>::reference g(T& t)
		{
			return tupxbase::g(t);
		}
	};

	template<typename T>
	class tuple_at<T,0>
	{
	public:
		typedef typename T::type0 type;
		static inline typename tuple_param<type>::reference g(T& t)
		{
			return t;
		}
	};

	template<typename T>
	class tuple_at<T,-1>
	{
	public:
		typedef nulltype type;
	};

	template<typename T,typename X>
	class tuple_id
	{
	public:
		static const int value9=tl::is_same_type<X,typename T::type9>::value?9:-1;
		static const int value8=tl::is_same_type<X,typename T::type8>::value?8:value9;
		static const int value7=tl::is_same_type<X,typename T::type7>::value?7:value8;
		static const int value6=tl::is_same_type<X,typename T::type6>::value?6:value7;
		static const int value5=tl::is_same_type<X,typename T::type5>::value?5:value6;
		static const int value4=tl::is_same_type<X,typename T::type4>::value?4:value5;
		static const int value3=tl::is_same_type<X,typename T::type3>::value?3:value4;
		static const int value2=tl::is_same_type<X,typename T::type2>::value?2:value3;
		static const int value1=tl::is_same_type<X,typename T::type1>::value?1:value2;
		static const int value0=tl::is_same_type<X,typename T::type0>::value?0:value1;
		static const int value=value0;
	};

	template<typename T,template<typename> class X>
	class tuple_mp
	{
	public:
		typedef tuple<
			typename X<typename T::type0>::type,
			typename X<typename T::type1>::type,
			typename X<typename T::type2>::type,
			typename X<typename T::type3>::type,
			typename X<typename T::type4>::type,
			typename X<typename T::type5>::type,
			typename X<typename T::type6>::type,
			typename X<typename T::type7>::type,
			typename X<typename T::type8>::type,
			typename X<typename T::type9>::type
		> type;

	};
// ----------------------------------------------------------------------------------------

	/*!A is_pointer_type

		This is a template where is_pointer_type<T>::value == true when T is a pointer
		type and false otherwise.
	!*/

	template <typename T>	class is_pointer_type : public value_type<false> {};
	template <typename T>   class is_pointer_type<T*> : public value_type<true> {};

// ----------------------------------------------------------------------------------------

	/*!A is_const_type

		This is a template where is_const_type<T>::value == true when T is a const
		type and false otherwise.
	!*/

	template <typename T> struct is_const_type : public value_type<false> {};
	template <typename T> struct is_const_type<const T> : public value_type<true> {};


// ----------------------------------------------------------------------------------------

	/*!A is_reference_type

		This is a template where is_reference_type<T>::value == true when T is a reference
		type and false otherwise.
	!*/

	template <typename T> struct is_reference_type : public value_type<false> {};
	template <typename T> struct is_reference_type<const T&>  : public value_type<true> {};
	template <typename T> struct is_reference_type<T&>  : public value_type<true> {};


// ----------------------------------------------------------------------------------------

	/*!A is_convertible

		This is a template that can be used to determine if one type is convertible
		into another type.

		For example:
			is_convertible<int,float>::value == true	// because ints are convertible to floats
			is_convertible<int*,float>::value == false  // because int pointers are NOT convertible to floats
	!*/
	namespace detail
	{
		template <typename from, typename to>
		struct is_convertible_helper
		{
			static char test(to&);
			static int test(...);
			const static bool value = sizeof(test(*(from*)0)) == sizeof(char);
		};
		template <typename from, typename to>
		struct is_convertible_helper<from,to*>
		{
			static char test(to*);
			static int test(...);
			const static bool value = sizeof(test(*(from*)0)) == sizeof(char);
		};
	}

	template <typename from, typename to>
	struct is_convertible : public value_type< detail::is_convertible_helper<from,to>::value > {};


// ----------------------------------------------------------------------------------------

	/*!A is_unsigned_type

		This is a template where is_unsigned_type<T>::value == true when T is an unsigned
		integral type and false when T is a signed integral type.
	!*/
	namespace detail
	{
		template <typename T>
		struct is_unsigned_type_helper
		{
			static const bool value = static_cast<T>((static_cast<T>(0)-static_cast<T>(1))) > 0;
		};
	}

	template <typename T> struct is_unsigned_type : public value_type< detail::is_unsigned_type_helper<T>::value > {};


// ----------------------------------------------------------------------------------------

	/*!A is_signed_type

		This is a template where is_signed_type<T>::value == true when T is a signed
		integral type and false when T is an unsigned integral type.
	!*/
	template <typename T> struct is_signed_type : public value_type< !detail::is_unsigned_type_helper<T>::value > {};



	template <bool B, class T = void> struct enable_if;

	template <class T> struct enable_if<true, T>
	{
		typedef T type;
	};
	template <class T> struct enable_if<false,T> {};


	template <class Cond, class T = void>
	struct enable_if_c : public enable_if<Cond::value, T> {};

	template <typename TYPELIST, typename T1,typename T2=T1>
	struct enable_tl : public enable_if< TYPELIST::template id<T1>::value>=0, T2> {};

	template <typename T1, typename TB,typename T2=void>
	struct enable_cv : public enable_if<is_convertible<T1,TB>::value,T2> {};

	template <bool B, class T = void>
	struct disable_if : public enable_if<!B,T> {};

	template <class Cond, class T = void>
	struct disable_if_c : public disable_if<Cond::value, T> {};

	template <typename TYPELIST, typename T1,typename T2=T1>
	struct disable_tl : public disable_if< TYPELIST::template id<T1>::value>=0, T2> {};

	template <typename T1, typename TB,typename T2=void>
	struct disable_cv : public disable_if<is_convertible<T1,TB>::value,T2> {};

	typedef mk_typelist<char,short,int,long,long long>::type signed_integer_type;
	typedef mk_typelist<unsigned char,unsigned short,unsigned int,unsigned long,unsigned long long>::type unsigned_integer_type;
	typedef signed_integer_type::cat<unsigned_integer_type>::type integer_type;

	typedef mk_typelist<float,double,long double>::type floating_type;


	template <typename T> struct is_scalar_type : public value_type<false> {};

	template <> struct is_scalar_type<float>		   : public value_type<true> {};
	template <> struct is_scalar_type<double>          : public value_type<true> {};
	template <> struct is_scalar_type<long double>     : public value_type<true> {};
	template <> struct is_scalar_type<short>           : public value_type<true> {};
	template <> struct is_scalar_type<int>             : public value_type<true> {};
	template <> struct is_scalar_type<long>            : public value_type<true> {};
	template <> struct is_scalar_type<unsigned short>  : public value_type<true> {};
	template <> struct is_scalar_type<unsigned int>    : public value_type<true> {};
	template <> struct is_scalar_type<unsigned long>   : public value_type<true> {};
	template <> struct is_scalar_type<long long>       : public value_type<true> {};
	template <> struct is_scalar_type<unsigned long long>       : public value_type<true> {};
	template <> struct is_scalar_type<char>            : public value_type<true> {};
	template <> struct is_scalar_type<wchar_t>         : public value_type<true> {};
	template <> struct is_scalar_type<signed char>     : public value_type<true> {};
	template <> struct is_scalar_type<unsigned char>   : public value_type<true> {};


// ----------------------------------------------------------------------------------------

	/*!A tabs

	    This is a template to compute the absolute value a number at compile time.

	    For example,
	        tabs<-4>::value == 4
	        tabs<4>::value == 4
	!*/

	template <long x, typename enabled=void>
	struct tabs
	{
		const static long value = x;
	};
	template <long x>
	struct tabs<x,typename enable_if<(x < 0)>::type>
	{
		const static long value = -x;
	};

// ----------------------------------------------------------------------------------------

	/*!A tmax

	    This is a template to compute the max of two values at compile time

	    For example,
	        tmax<4,7>::value == 7
	!*/

	template <long x, long y, typename enabled=void>
	struct tmax
	{
		const static long value = x;
	};
	template <long x, long y>
	struct tmax<x,y,typename enable_if<(y > x)>::type>
	{
		const static long value = y;
	};

// ----------------------------------------------------------------------------------------

	/*!A tmin

	    This is a template to compute the min of two values at compile time

	    For example,
	        tmin<4,7>::value == 4
	!*/

	template <long x, long y, typename enabled=void>
	struct tmin
	{
		const static long value = x;
	};
	template <long x, long y>
	struct tmin<x,y,typename enable_if<(y < x)>::type>
	{
		const static long value = y;
	};



// ----------------------------------------------------------------------------------------


	template<typename T>
	class type_trait_base
	{
	public:

		typedef T type;
		typedef T* pointer;
		typedef const T* const_pointer;
		typedef T base_type;
		typedef nulltype elem_type;
		typedef const T const_type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef typename meta_if<is_scalar_type<T>::value,const_type,const_reference>::type param_type;
		typedef const_type return_type;

	};


	template<typename T>
	class type_trait : public type_trait_base<T>
	{
	public:

	};

	template<typename T>
	class type_trait<const T> : public type_trait<T>
	{
	public:
		typedef const T type;
	};

	template<typename T>
	class type_trait<T&> : public type_trait<T>
	{
	public:
		typedef T& type;
	};

	template<typename T>
	class type_trait<T*> : public type_trait<T>
	{
	public:
		typedef T* type;
	};

	template<typename T>
	struct is_pod : public is_scalar_type<T> {};

	template<>
	struct is_pod<bool> : public value_type<true> {};

}

#endif
