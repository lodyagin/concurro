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

#include "stdafx.h"
#include "SWinCheck.h"

namespace curr {

void sWinCheck (BOOL ok)
{
  sWinCheck (ok, L"");
}

void sWinCheck( BOOL ok, const wchar_t * fmt, ... )
{
  if ( ok ) return;

  va_list list;
  va_start(list, fmt);
  wstring str(sFormatVa(fmt, list));
  va_end(list);

  throw SException(L"Error "+str+L": "+sWinErrMsg(GetLastError()));
}

void sWinError( const wchar_t * fmt, ... )
{
  va_list list;
  va_start(list, fmt);
  wstring str(sFormatVa(fmt, list));
  va_end(list);

  throw SException(L"Error "+str+L": "+sWinErrMsg(GetLastError()));
}

void sWinErrorCode( DWORD code, const wchar_t * fmt, ... )
{
  va_list list;
  va_start(list, fmt);
  wstring str(sFormatVa(fmt, list));
  va_end(list);

  throw SException(L"Error "+str+L": "+sWinErrMsg(code));
}

}

