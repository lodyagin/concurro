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

#define THROW_EXCEPTION(exception_class, stream_expr) { \
  std::wostringstream oss_; \
  { stream_expr ; } \
  oss_ << L" at " << _T(__FILE__) << L':' << __LINE__ \
       << L", " << _T(__FUNCTION__); \
       throw exception_class(oss_.str()); \
}


// user mistake - wrong action, invalid configuration etc
class SUserError : public SException
{
public:

  typedef SException Parent;

  SUserError( const std::string & what ) : Parent(what) {}
  SUserError( const std::wstring & what ) : Parent(what) {}

};


SMAKE_THROW_FN_DECL(sUserError, SUserError)


