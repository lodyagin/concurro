#ifndef __SEXCEPTION_H
#define __SEXCEPTION_H

#include "SCommon.h"
#include <exception>


// base for the all SCommon exceptions

class SException : public std::exception
{
public:

  explicit SException (const string & what, bool alreadyLogged = false);
  virtual ~SException();

  bool isAlreadyLogged () const  { return alreadyLoggedFlag; }

  virtual const char * what() const;

protected:
  string _what;
  bool alreadyLoggedFlag;
};

//extern const SException ProgramError;
//extern const SException NotImplemented;

#define THROW_EXCEPTION(exception_class, stream_expr) { \
  std::ostringstream oss_; \
  { stream_expr ; } \
  oss_ << " at " << __FILE__ << ':' << __LINE__ \
       << ", " << __FUNCTION__; \
       throw exception_class(oss_.str()); \
}


// user mistake - wrong action, invalid configuration etc
class SUserError : public SException
{
public:

  typedef SException Parent;

  SUserError( const string & what ) : Parent(what) {}

};


SMAKE_THROW_FN_DECL(sUserError, SUserError)


#endif  // __SEXCEPTION_H
