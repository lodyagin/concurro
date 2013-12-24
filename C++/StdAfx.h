/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

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

#include <iostream>
#include "Logging.h"
//#include "RCheck.h"
#include "SException.h"
//#include "logging.h"
//#include "SCommon.h"
#ifdef _WIN32
#include "SWinCheck.h"
#endif
//#include "RMutex.h"
//#include "RState.hpp"
//#include "REvent.hpp"
//#include "SShutdown.h"
//#include "SCheck.h"

#include <string>
//#include <assert.h> better use SCheck
