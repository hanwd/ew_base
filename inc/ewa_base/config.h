#ifndef __H_EW_EWA_BASE_CONFIG__
#define __H_EW_EWA_BASE_CONFIG__

#include "tl.h"

#define EW_C11

#ifdef EW_C11
	#define EW_MOVE(x) std::move(x)
	#define EW_RVALUE(T) T&&
	#define EW_FORWARD(T,x) std::forward<T>(x)
#else
	#define EW_MOVE(x) x
	#define EW_RVALUE(T) const T&
	#define EW_FORWARD(T,x) x
#endif

// use MemPoolPaging as default memory allocator
//#define EW_MEMUSEPOOL


#define EW_OBJECT_USE_MP_ALLOC

//#define EW_NO64BIT_ATOMIC 1

#if defined(_WIN64)
	#define EW_WIN64
	#define EW_X64
	#define EW_WINDOWS
#elif defined(_WIN32)
	#define EW_WIN32
	#define EW_X86
	#define EW_WINDOWS
#else
	#ifdef __LP64__
		#define EW_X64
	#else
		#define EW_X86
	#endif
	#define EW_LINUX
#endif

#ifdef _MSC_VER
    #define EW_MSVC
#endif


#define EW_UNUSED(X) ((void)&(X));


#ifdef _MSC_VER
#define EW_ATTRIBUTE(X) __declspec(X)
#else
#define EW_ATTRIBUTE(X) __attribute__((X))
#endif

#ifdef _MSC_VER
#define EW_THREAD_TLS __declspec(thread)
#else
#define EW_THREAD_TLS __thread
#endif

#ifdef EWA_BASE_DLL
#ifdef _MSC_VER
#define EWA_BASE_EXPORT __declspec(dllexport)
#define EWA_BASE_IMPORT __declspec(dllimport)
#else
#define EWA_BASE_EXPORT __attribute__((dllexport))
#define EWA_BASE_IMPORT __attribute__((dllimport))
#endif
#else
#define EWA_BASE_EXPORT
#define EWA_BASE_IMPORT
#endif

#ifdef _MSC_VER
#pragma warning(disable:4251)
#endif


#if !defined(NDEBUG)
#define EW_FORCEINLINE inline
#elif defined(EW_MSVC)
#define EW_FORCEINLINE __forceinline
#elif defined(EW_LINUX)
#define EW_FORCEINLINE inline __attribute__((always_inline))
#else
#define EW_FORCEINLINE inline
#endif

#ifdef EWA_BASE_BUILDING
#define DLLIMPEXP_EWA_BASE EWA_BASE_EXPORT
#else
#define DLLIMPEXP_EWA_BASE EWA_BASE_IMPORT
#endif

//#define EW_CHECK_HEAP

#ifdef _DEBUG
	//detect heap corruption for mp_alloc/mp_free
	#define EW_CHECK_HEAP
	#define EW_CHECK_HEAP_BREAK
#endif

#ifdef _MSC_VER

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;


#ifndef _UINTPTR_T_DEFINED
#ifdef  _WIN64
typedef uint64_t uintptr_t;
typedef int64_t intptr_t;
#else
typedef uint32_t uintptr_t;
typedef int32_t intptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#else
#include <sys/types.h>
#include <stdint.h>
#endif


template<int> struct unsigned_integer_type;
template<> struct unsigned_integer_type<1>{typedef uint8_t type;};
template<> struct unsigned_integer_type<2>{typedef uint16_t type;};
template<> struct unsigned_integer_type<4>{typedef uint32_t type;};
template<> struct unsigned_integer_type<8>{typedef uint64_t type;};


typedef float float32_t;
typedef double float64_t;


#ifndef _kT
#define _kT(x) (x)
#endif

#ifndef _hT
#define _hT(x) ew::Translate(x)
#endif

#ifndef _zT
#define _zT(x) ew::Translate(x,"")
#endif


#if defined(EW_MEMDEBUG) || defined(EW_MEMUSEPOOL)

void* operator new(size_t);
void* operator new(size_t,const char*,int);
void* operator new(size_t,int,const char*,int);
void* operator new[](size_t);
void* operator new[](size_t,const char*,int);
void* operator new[](size_t,int,const char*,int);

void operator delete(void*);
void operator delete(void*,const char*,int);
void operator delete(void*,int,const char*,int);
void operator delete[](void*);
void operator delete[](void*,const char*,int);
void operator delete[](void*,int,const char*,int);

#endif


#if defined(EW_MEMDEBUG)

#define new DEBUG_NEW
#define DEBUG_NEW  new (__FILE__,__LINE__)

#endif

#define DEFINE_OBJECT_NAME(TYPE,DESC)\
	template<> class ObjectNameT<TYPE>{public:\
		static String MakeName(const String& s){return (s+"#")+DESC;} \
		static String MakeName(){return DESC;} \
	};

#define DEFINE_OBJECT_NAME_T(TYPE,DESC)\
	template<typename T> class ObjectNameT<TYPE<T> >{public:static String MakeName(const String& s){return ObjectNameT<T>::MakeName((s+"#")+DESC);} };


#define EW_ENTER namespace ew{
#define EW_LEAVE };

#ifndef NDEBUG
#define EW_ASSERT(x) if(!(x)) ew::OnAssertFailure(#x,__FILE__,__LINE__);
#define EW_STATIC_ASSERT(x) {char buffer[(x)?1:0];(void)&buffer;}
#else
#define EW_ASSERT(x)
#define EW_STATIC_ASSERT(x)
#endif


#ifdef _DEBUG
#define EW_ASSERT_INDEX_VALID(X,Y) EW_ASSERT(size_t(X)<size_t(Y))
#else
#define EW_ASSERT_INDEX_VALID(X,Y)
#endif


#ifndef NULL
#ifdef __cplusplus
#define NULL	0
#else
#define NULL	((void *)0)
#endif
#endif


EW_ENTER

// Prevent form copy
class DLLIMPEXP_EWA_BASE NonCopyable
{
	NonCopyable(const NonCopyable&);
	NonCopyable& operator=(const NonCopyable&);
protected:
	NonCopyable() {}
	~NonCopyable() {}
};

template<typename T> class ObjectNameT;

#pragma push_macro("new")
#undef new

// class mp_obj use MemPool to allocate and deallocate memory
class DLLIMPEXP_EWA_BASE mp_obj
{
public:

	static void* operator new(size_t);
	static void operator delete(void*);
	static void* operator new(size_t,const char*,int);
	static void operator delete(void*,const char*,int);
	static void* operator new(size_t,void* p){return p;}
	static void operator delete(void*,void*){}

	static void* operator new[](size_t);
	static void operator delete[](void*);
	static void* operator new[](size_t,const char*,int);
	static void operator delete[](void*,const char*,int);
	static void* operator new[](size_t,void* p){return p;}
	static void operator delete[](void*,void*){}

};

#pragma pop_macro("new")


class DLLIMPEXP_EWA_BASE String;
class DLLIMPEXP_EWA_BASE TimeSpan;
class DLLIMPEXP_EWA_BASE TimePoint;

DLLIMPEXP_EWA_BASE extern const char const_empty_buffer[64];

void DLLIMPEXP_EWA_BASE OnAssertFailure(const char* what,const char* file,long line);

DLLIMPEXP_EWA_BASE const String& Translate(const String& msg);
DLLIMPEXP_EWA_BASE const String& Translate(const String& msg,const String& def);

enum
{
	LOGLEVEL_DEBUG,
	LOGLEVEL_INFO,
	LOGLEVEL_TRACE,
	LOGLEVEL_PRINT,
	LOGLEVEL_PRINTLN,
	LOGLEVEL_MESSAGE,
	LOGLEVEL_COMMAND,
	LOGLEVEL_USER,
	LOGLEVEL_WARNING=20,
	LOGLEVEL_ERROR,
	LOGLEVEL_FATAL,
	LOGLEVEL_MAX,
};

template<typename T>
class hash_t;


class DLLIMPEXP_EWA_BASE Serializer;

template<typename T>
Serializer& operator &(Serializer& ar,T& val);

EW_LEAVE

#endif
