#include "stdafx.h"
#include "SWinCheck.h"

void sWinCheck (BOOL ok)
{
  sWinCheck (ok, "");
}

void sWinCheck( BOOL ok, const char * fmt, ... )
{
  if ( ok ) return;

  va_list list;
  va_start(list, fmt);
  string str(sFormatVa(fmt, list));
  va_end(list);

  throw SException("Error "+str+": "+sWinErrMsg(GetLastError()));
}

void sSocketCheck( BOOL ok )
{
  sSocketCheck (ok , "");
}

void sSocketCheck( BOOL ok, const char * fmt, ... )
{
  if ( ok ) return;

  va_list list;
  va_start(list, fmt);
  string str(sFormatVa(fmt, list));
  va_end(list);

  throw SException("Error "+str+": "+sWinErrMsg(WSAGetLastError()));
}

void sWinError( const char * fmt, ... )
{
  va_list list;
  va_start(list, fmt);
  string str(sFormatVa(fmt, list));
  va_end(list);

  throw SException("Error "+str+": "+sWinErrMsg(GetLastError()));
}

void sWinErrorCode( DWORD code, const char * fmt, ... )
{
  va_list list;
  va_start(list, fmt);
  string str(sFormatVa(fmt, list));
  va_end(list);

  throw SException("Error "+str+": "+sWinErrMsg(code));
}


