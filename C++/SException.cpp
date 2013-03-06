#include "StdAfx.h"
#include "SException.h"
#include "SCommon.h"
#include <typeinfo>

// SException  ==================================================================

#ifdef _WIN32
SException::SException (const std::wstring & w, bool alreadyLogged) :
  whatU(w), alreadyLoggedFlag (alreadyLogged)
{
  _what = wstr2str (whatU);
   if (!alreadyLogged)
     LOG4CXX_DEBUG (Logging::Root (), std::wstring (L"SException: ") + w);
}
#endif

SException::SException (const std::string & w) :
  _what(w)//, alreadyLoggedFlag (alreadyLogged)
{
#ifdef _WIN32
  whatU = str2wstr (_what);
#endif
}

SException::~SException() throw ()
{
}

const char * SException::what() const throw ()
{
  return _what.c_str();
}

//const SException ProgramError ("Program Error");
//extern const SException NotImplemented ("Not Implemented");

#ifdef _WIN32
SMAKE_THROW_FN_IMPL(throwSException, SException)
SMAKE_THROW_FN_IMPL(sUserError, SUserError)
#endif

std::ostream& operator<< (std::ostream& out, const SException& exc)
{
  out << typeid(exc).name() << ": " << exc.what();
  return out;
}
