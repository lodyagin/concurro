#include "stdafx.h"
#include "SCheck.h"
#include <stdarg.h>


void sWarn( bool cond, const char * fmt, ... )
{
  if ( !cond ) return;

  va_list va;
  va_start(va, fmt);
  OutputDebugStringA(sFormatVa(fmt, va).c_str());
  va_end(va);
}

void sTrace( const char * fmt, ... )
{
  va_list va;
  va_start(va, fmt);
  OutputDebugStringA(sFormatVa(fmt, va).c_str());
  va_end(va);
}
