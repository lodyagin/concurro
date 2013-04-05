#ifndef CONCURRO_RCHECK_H_
#define CONCURRO_RCHECK_H_

#include "SCommon.h"

#ifdef _WIN32
void rCheck( BOOL );
void rCheck( BOOL, const wchar_t * fmt, ... );
void rError( const wchar_t * fmt, ... );
void rErrorCode( DWORD code, const wchar_t * fmt, ... );
#else
#define rCheck rSocketCheck
#endif

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
  THROW_EXCEPTION (SException, \
						 SFORMAT("Socket error : " << sWinErrMsg(WSAGetLastError()))) \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    int errsav = errno; \
    THROW_EXCEPTION (SException, \
							SFORMAT("Socket error : " << strerror(errsav)));	\
		} \
}
#endif

#ifdef _WIN32
#define rSocketCheckWithMsg(cond, msg) \
{ \
  if (!(cond)) \
  THROW_EXCEPTION (SException, \
	 SFORMAT("Socket error : " << sWinErrMsg(WSAGetLastError()) \
				<< msg))															\
}
#else
#define rSocketCheckWithMsg(cond, msg)							\
{ \
  if (!(cond)) {	 \
    int errsav = errno; \
    THROW_EXCEPTION (SException, \
							SFORMAT("Socket error : " << strerror(errsav) << msg))	\
		}\
}
#endif



#endif  // __SWINCHECK_H
