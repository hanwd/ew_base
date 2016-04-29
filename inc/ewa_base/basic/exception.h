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
	static void XError();
	static void XError(const String& s);
	static void XError(const char* p, bool copy_);

	// Throw std::bad_alloc
	static void XBadAlloc();

	static void XBadCast();
	static void XInvalidArgument();
	static void XInvalidIndex();
	static void XNotFound();
	static void XReadOnly();
	static void XTypeLocked();
	static void XBadFunctor();
	

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
