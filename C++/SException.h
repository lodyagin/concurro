#pragma once

#include "SCommon.h"
#include "HasStringView.h"
#include <exception>
#include <ostream>

// base for the all SCommon exceptions

class SException : public std::exception, public HasStringView
{
public:

  explicit SException (const std::string & what/*, bool alreadyLogged = false*/);
  explicit SException (const std::wstring & what/*, bool alreadyLogged = false*/);
  virtual ~SException() throw();

  bool isAlreadyLogged () const  { return alreadyLoggedFlag; }

  virtual const char * what() const throw();

  void outString(std::ostream& out) const
  {
	 out << this->what();
  }

protected:
#ifdef _WIN32
  std::wstring whatU;
#endif
  std::string _what;
  bool alreadyLoggedFlag;
};

#define THROW_EXCEPTION(exception_class, par) { \
	 exception_class exc_(par);								  \
  LOG_DEBUG(Logger<LOG::Root>, "Throw " << exc_); \
  throw exc_; \
  } while (0)

#define THROW_PROGRAM_ERROR \
  THROW_EXCEPTION(SException, "Program Error")

#define THROW_NOT_IMPLEMENTED \
  THROW_EXCEPTION(SException, "Not implemented")

// user mistake - wrong action, invalid configuration etc
class SUserError : public SException
{
public:

  typedef SException Parent;

  SUserError( const std::string & what ) : Parent(what) {}
  SUserError( const std::wstring & what ) : Parent(what) {}

};


SMAKE_THROW_FN_DECL(sUserError, SUserError)

std::ostream& operator<< (std::ostream&, const SException& exc);

#define DEFINE_EXCEPTION(class_, msg) \
class class_ : public SException \
{ \
public: \
  class_() : SException(msg) {} \
};
