#include "stdafx.h"
#include "SException.h"


// SException  ==================================================================

SException::SException (const string & w, bool alreadyLogged) :
  _what(w), alreadyLoggedFlag (alreadyLogged)
{
   if (!alreadyLogged)
     LOG4CXX_DEBUG (Logging::Root (), std::string ("SException: ") + w);
}

SException::~SException()
{
}

const char * SException::what() const
{
  return _what.c_str();
}

const SException ProgramError ("Program Error");
extern const SException NotImplemented ("Not Implemented");

SMAKE_THROW_FN_IMPL(throwSException, SException)
SMAKE_THROW_FN_IMPL(sUserError, SUserError)
