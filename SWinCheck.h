#ifndef __SWINCHECK_H
#define __SWINCHECK_H

#include "SCommon.h"


void sWinCheck( BOOL );
void sWinCheck( BOOL, const wchar_t * fmt, ... );
void sWinError( const wchar_t * fmt, ... );
void sWinErrorCode( DWORD code, const wchar_t * fmt, ... );

#define sSocketCheck(cond) \
{ \
  if (!(cond)) \
  THROW_EXCEPTION (SException, \
     oss_ << L"Socket error : " << sWinErrMsg(WSAGetLastError())) \
}

#define sSocketCheckWithMsg(cond, msg) \
{ \
  if (!(cond)) \
  THROW_EXCEPTION (SException, \
     oss_ << "Socket error : " << sWinErrMsg(WSAGetLastError()) \
          << msg) \
}


#endif  // __SWINCHECK_H
