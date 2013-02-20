#pragma once
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // without winsock 1
#include <windows.h>
#include <Ws2tcpip.h>
#include <Winsock2.h>

#include "Logging.h"
#include "SWinCheck.h"
#include "SException.h"
#include "logging.h"
#include "SCommon.h"
#include "SWinCheck.h"
#include "SEvent.h"
#include "SMutex.h"
#include "SShutdown.h"

#include <string>
#include <assert.h>

