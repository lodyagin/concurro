#include "StdAfx.h"
#include "SException.h"
#include "SCommon.h"


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

SException::SException (const std::string & w, bool alreadyLogged) :
  _what(w), alreadyLoggedFlag (alreadyLogged)
{
#ifdef _WIN32
  whatU = str2wstr (_what);
#endif
   if (!alreadyLogged)
     LOG_DEBUG (Logger<LOG::Root>, "SException: " << w);
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
