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

class RSocketError : public SException
{
public:
  const int error_code;

  RSocketError(int err_code, 
					const std::string& msg = std::string())
  : SException(SFORMAT("Socket error : " 
#ifdef _WIN32
							  << sWinErrMsg(err_code)
#else
							  << strerror(err_code)
#endif
					)),
	 error_code(err_code) {}
};

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSocketError, WSAGetLastError()); \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSocketError, errno); \
  } \
}
#endif

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSocketError, WSAGetLastError()); \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSocketError, errno); \
  } \
}
#endif

#ifdef _WIN32
#define rSocketCheckWithMsg(cond, msg)					\
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSocketError, WSAGetLastError(),msg);\
}
#else
#define rSocketCheckWithMsg(cond, msg)				\
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSocketError, errno, msg);		\
  } \
}
#endif

#endif  // __SWINCHECK_H
