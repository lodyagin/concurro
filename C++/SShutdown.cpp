/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
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

#include "StdAfx.h"
#include "SShutdown.h"

namespace curr {

#ifdef _WIN32
// SShutdown  ========================================================

SShutdown::SShutdown() :
  evt(CreateEvent(0, true, false, 0))
{             // SA manual  init name
  sWinCheck(evt != 0, L"creating event");
  complete_construction();
}

SShutdown::~SShutdown()
{
  CloseHandle(evt);
}

void SShutdown::shutdown()
{
  sWinCheck(SetEvent(evt), L"Setting event");

  for ( size_t i = 0; i < ports.size(); ++i )
    ports[i]->postEmptyEvt();
}

bool SShutdown::isShuttingDown()
{
  return WaitForSingleObject(evt, 0) == WAIT_OBJECT_0;
}

void SShutdown::registerComplPort( SComplPort & port )
{
  ports.push_back(&port);
}

void SShutdown::unregisterComplPort( SComplPort & port )
{
  for ( size_t i = 0; i < ports.size(); ++i )
    if ( ports[i] == &port ) 
    {
      ports.erase(ports.begin() + i);
      return;
    }
  SCHECK(0);  // unregistering non-registered port
}
#endif

#if 1
// XShuttingDown  ====================================================

XShuttingDown::XShuttingDown( const std::string & act ) :
  Parent(SFORMAT(act << " action interrupted by a shutdown signal")),
  _action(act)
{
}


//====================================================================

void xShuttingDown( const std::string & act )
{
  throw XShuttingDown(act);
}
#endif

#ifdef _WIN32
void sCheckShuttingDown()
{
  if ( SSHUTDOWN.isShuttingDown() ) xShuttingDown();
}

#endif
}
