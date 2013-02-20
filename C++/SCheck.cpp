#include "stdafx.h"
#include "SCheck.h"
#include <stdarg.h>


void sWarn( bool cond, const wchar_t * fmt, ... )
{
  if ( !cond ) return;

  va_list va;
  va_start(va, fmt);
  OutputDebugString(sFormatVa(fmt, va).c_str());
  va_end(va);
}

void sTrace( const wchar_t * fmt, ... )
{
  va_list va;
  va_start(va, fmt);
  OutputDebugString(sFormatVa(fmt, va).c_str());
  va_end(va);
}
