#ifndef __H_EW_BASIC_EXCEPTION__
#define __H_EW_BASIC_EXCEPTION__

#include "ewa_base/config.h"
#include <stdexcept>

EW_ENTER

class DLLIMPEXP_EWA_BASE String;

class DLLIMPEXP_EWA_BASE Exception : public std::exception
{
public:

	Exception();
	Exception(const Exception& o);
	Exception(const char* p,bool copy_=true);
	Exception(const String& p);
	Exception& operator=(const Exception& o);

	~Exception() throw();

	// Throw Exception
	static EW_NORETURN void XError();
	static EW_NORETURN void XError(const String& s);
	static EW_NORETURN void XError(const char* p, bool copy_);

	// Throw std::bad_alloc
	static EW_NORETURN void XBadAlloc();

	static EW_NORETURN void XBadCast();
	static EW_NORETURN void XInvalidArgument();
	static EW_NORETURN void XInvalidIndex();
	static EW_NORETURN void XNotFound();
	static EW_NORETURN void XReadOnly();
	static EW_NORETURN void XTypeLocked();
	static EW_NORETURN void XBadFunctor();
	

#ifdef EW_WINDOWS
	const char* what() const throw();
#else
	const char* what() throw();
#endif

protected:
	const char* m_sWhat;
	bool m_bDelete;
};


EW_LEAVE
#endif
