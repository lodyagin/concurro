/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RCHECK_H_
#define CONCURRO_RCHECK_H_

#include "SCommon.h"

#ifdef _WIN32
void rCheck( BOOL );
void rCheck( BOOL, const wchar_t * fmt, ... );
void rError( const wchar_t * fmt, ... );
void rErrorCode( DWORD code, const wchar_t * fmt, ... );
#define rErrorMsg(err) sWinErrMsg(err)
#else
#define rErrorMsg(err) strerror(err)
#define rCheck rSocketCheck
#endif

class RSystemError : public SException
{
public:
  const int error_code;

  RSystemError(int err_code, 
					const std::string& msg = std::string())
  : SException(SFORMAT("System error : " 
							  << rErrorMsg(err_code) << ' ')),
	 error_code(err_code) {}
};

class RSocketError : public RSystemError
{
public:
  RSocketError(int err_code, 
					const std::string& msg = std::string())
	 : RSystemError(err_code, msg) {}
};

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSystemError, WSAGetLastError()); \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSystemError, errno); \
  } \
}
#endif

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSystemError, WSAGetLastError()); \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSystemError, errno); \
  } \
}
#endif

#ifdef _WIN32
#define rSocketCheckWithMsg(cond, msg)					\
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSystemError, WSAGetLastError(),msg);\
}
#else
#define rSocketCheckWithMsg(cond, msg)				\
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSystemError, errno, msg);		\
  } \
}
#endif

#endif  // __SWINCHECK_H
