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

#include "ServerSocket.h"

namespace curr {

/*DEFINE_AXIS(
  ServerSocketAxis,
  {},
  {}
);*/

ServerSocket::ServerSocket
  (const ObjectCreationInfo& oi, 
   const RSocketAddress& par)
 : 
   RSocketBase(oi, par)/*,
   RStateSplitter<ServerSocketAxis, SocketBaseAxis>
     (this, createdState,
      RStateSplitter<ServerSocketAxis, SocketBaseAxis>
      ::state_hook(&ServerSocket::state_hook)
     )*//*,
   thread(dynamic_cast<Thread*>
          (RSocketBase::repository->thread_factory
           -> create_thread(Thread::Par(this))))*/
{
   //SCHECK(thread);
   //RStateSplitter<ServerSocketAxis, SocketBaseAxis>::init();
   //this->RSocketBase::threads_terminals.push_back
   //   (thread->is_terminal_state());
}

ServerSocket::~ServerSocket()
{
   LOG_DEBUG(log, "~ServerSocket()");
}


}
