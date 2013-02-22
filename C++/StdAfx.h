#pragma once
#ifdef _WIN32
#  include "targetver.h"

#  define WIN32_LEAN_AND_MEAN // without winsock 1
#  include <windows.h>
#  include <Ws2tcpip.h>
#  include <Winsock2.h>
#else
#  define _SVID_SOURCE // for putenv
#  include <stdlib.h>
#endif

#include "Logging.h"
#ifdef _WIN32
#include "SWinCheck.h"
#endif
#include "SException.h"
//#include "logging.h"
#include "SCommon.h"
#ifdef _WIN32
#include "SWinCheck.h"
#include "SEvent.h"
#include "SMutex.h"
#endif
#include "SShutdown.h"

#include <string>
#include <assert.h>

