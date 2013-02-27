#ifndef __SCHECK_H
#define __SCHECK_H

#include "SCommon.h"
#include "SException.h"


#define SCHECK(val) {		\
  if (!(val)) { \
  std::ostringstream oss_; \
  oss_ << "SCHECK(" #val ") failed" \
       << " at " << (__FILE__) << ':' << __LINE__		\
       << ", " << (__FUNCTION__); \
       throw SException(oss_.str()); \
  }} while (0)

#define SPRECONDITION(val) SCHECK(val)

#ifdef _WIN32
#define SWARN   sWarn
#endif

#ifdef __STRACE 
#define STRACE sTrace
#else
#define STRACE sNullTrace
#endif


void sWarn( bool cond, const wchar_t * fmt, ... );
void sTrace( const wchar_t * fmt, ... );

inline void sNullTrace(...) {}


#endif  // __SCHECK_H
