#include "stdafx.h"
#include "SWinCheck.h"

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


