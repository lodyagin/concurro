#ifndef __SWINCHECK_H
#define __SWINCHECK_H

#include "SCommon.h"


void sWinCheck( BOOL );
void sWinCheck( BOOL, const char * fmt, ... );
void sSocketCheck( BOOL );
void sSocketCheck( BOOL, const char * fmt, ... );
void sWinError( const char * fmt, ... );
void sWinErrorCode( DWORD code, const char * fmt, ... );
//void sSocketCheck (BOOL, const char * fmt, ...);

#endif  // __SWINCHECK_H
