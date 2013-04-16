#pragma once

#define SHUTDOWN_UNIMPL

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
#include "RCheck.h"
#include "SException.h"
//#include "logging.h"
#include "SCommon.h"
#ifdef _WIN32
#include "SWinCheck.h"
#endif
#include "RMutex.h"
#include "RState.hpp"
#include "REvent.hpp"
#include "SShutdown.h"
#include "SCheck.h"

#include <string>
//#include <assert.h> better use SCheck
