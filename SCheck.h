#ifndef __SCHECK_H
#define __SCHECK_H

#include "SCommon.h"
#include <assert.h>


#define SCHECK(val)        assert(val);
#define SPRECONDITION(val) assert(val);

#define SWARN   sWarn

#ifdef __STRACE 
#define STRACE sTrace
#else
#define STRACE sNullTrace
#endif


void sWarn( bool cond, const char * fmt, ... );
void sTrace( const char * fmt, ... );

inline void sNullTrace(...) {}


#endif  // __SCHECK_H
