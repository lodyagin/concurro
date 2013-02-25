#pragma once

#include "SCommon.h"
#include <exception>


// base for the all SCommon exceptions

class SException : public std::exception
{
public:

  explicit SException (const std::string & what, bool alreadyLogged = false);
  explicit SException (const std::wstring & what, bool alreadyLogged = false);
  virtual ~SException() throw();

  bool isAlreadyLogged () const  { return alreadyLoggedFlag; }

  virtual const char * what() const throw();

protected:
#ifdef _WIN32
  std::wstring whatU;
#endif
  std::string _what;
  bool alreadyLoggedFlag;
};

#ifdef _WIN32
#  define THROW_EXCEPTION(exception_class, stream_expr) { \
  std::wostringstream oss_; \
  { stream_expr ; } \
  oss_ << L" at " << L(__FILE__) << L':' << __LINE__						 \
       << L", " << L(__FUNCTION__); \
       throw exception_class(oss_.str()); \
  } while (0)
#else
#define THROW_EXCEPTION(exception_class, stream_expr) { \
  std::ostringstream oss_; \
  { stream_expr ; } \
  oss_ << " at " << (__FILE__) << ':' << __LINE__						 \
       << ", " << (__FUNCTION__); \
       throw exception_class(oss_.str()); \
  } while (0)
#endif

#define THROW_PROGRAM_ERROR \
  THROW_EXCEPTION(SException, oss_ << "Program Error")

// user mistake - wrong action, invalid configuration etc
class SUserError : public SException
{
public:

  typedef SException Parent;

  SUserError( const std::string & what ) : Parent(what) {}
  SUserError( const std::wstring & what ) : Parent(what) {}

};


SMAKE_THROW_FN_DECL(sUserError, SUserError)


