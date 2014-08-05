/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "SException.h"
#include "SCommon.h"
#include <typeinfo>

#error Deprecated

namespace curr {

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
  out << "SException: " << exc.what();
  return out;
}

}
