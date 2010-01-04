#ifndef __SWINCHECK_H
#define __SWINCHECK_H

#include "SCommon.h"


void sWinCheck( BOOL );
void sWinCheck( BOOL, const char * fmt, ... );
void sWinError( const char * fmt, ... );
void sWinErrorCode( DWORD code, const char * fmt, ... );

#define sSocketCheck(cond) \
{ \
  if (!(cond)) \
  THROW_EXCEPTION (SException, \
     oss_ << "Socket error : " << sWinErrMsg(WSAGetLastError())) \
}

#define sSocketCheckWithMsg(cond, msg) \
{ \
  if (!(cond)) \
  THROW_EXCEPTION (SException, \
     oss_ << "Socket error : " << sWinErrMsg(WSAGetLastError()) \
          << msg) \
}


#endif  // __SWINCHECK_H
