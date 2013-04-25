#ifndef CONCURRO_RCHECK_H_
#define CONCURRO_RCHECK_H_

#include "SCommon.h"

#ifdef _WIN32
void rCheck( BOOL );
void rCheck( BOOL, const wchar_t * fmt, ... );
void rError( const wchar_t * fmt, ... );
void rErrorCode( DWORD code, const wchar_t * fmt, ... );
#define rErrorMsg(err) sWinErrMsg(err)
#else
#define rErrorMsg(err) strerror(err)
#define rCheck rSocketCheck
#endif

class RSystemError : public SException
{
public:
  const int error_code;

  RSystemError(int err_code, 
					const std::string& msg = std::string())
  : SException(SFORMAT("System error : " 
							  << rErrorMsg(err_code) << ' ')),
	 error_code(err_code) {}
};

class RSocketError : public RSystemError
{
public:
  RSocketError(int err_code, 
					const std::string& msg = std::string())
	 : RSystemError(err_code, msg) {}
};

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSystemError, WSAGetLastError()); \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSystemError, errno); \
  } \
}
#endif

#ifdef _WIN32
#define rSocketCheck(cond) \
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSystemError, WSAGetLastError()); \
}
#else
#define rSocketCheck(cond) \
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSystemError, errno); \
  } \
}
#endif

#ifdef _WIN32
#define rSocketCheckWithMsg(cond, msg)					\
{ \
  if (!(cond)) \
	 THROW_EXCEPTION(RSystemError, WSAGetLastError(),msg);\
}
#else
#define rSocketCheckWithMsg(cond, msg)				\
{ \
  if (!(cond)) {	 \
    THROW_EXCEPTION (RSystemError, errno, msg);		\
  } \
}
#endif

#endif  // __SWINCHECK_H
