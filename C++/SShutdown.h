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

#ifndef CONCURRO_SSHUTDOWN_H
#define CONCURRO_SSHUTDOWN_H

#ifdef _WIN32
#include "SComplPort.h"
#endif
#include "SSingleton.h"
#include "SException.h"
#include "SCommon.h"
#include <vector>

namespace curr {

#ifdef _WIN32
#define SSHUTDOWN  SShutdown::instance()


class SShutdown final : public SSingleton<SShutdown>
{
public:

  SShutdown();
  ~SShutdown();

  void shutdown();  // shutdown application (or current thread)
  bool isShuttingDown();

  HANDLE event() { return evt; }

  void registerComplPort( SComplPort & );
  void unregisterComplPort( SComplPort & );

private:

  friend class SSocket;

  HANDLE evt;
  std::vector<SComplPort *> ports;

};
#endif

//! @addtogroup exceptions
//! @{

class XShuttingDown : public SException
{
public:

  typedef SException Parent;

  explicit XShuttingDown( const std::string & interruptedAction = "unknown" );
  ~XShuttingDown () throw () {};

private:

  std::string _action;

};

//! @}

// throw XShuttindDown
void xShuttingDown( const std::string & interruptedAction = "unknown" );
void sCheckShuttingDown();  // throws ZSD if is shuttind down

}
#endif
