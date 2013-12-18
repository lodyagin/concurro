/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocketConnection.hpp"
#include "REvent.hpp"
#include "RState.hpp"
#include "RWindow.hpp"
#include "RThreadRepository.hpp"

namespace curr {

std::ostream&
operator<< (std::ostream& out, const RSocketConnection& c)
{
  out << "<connection>";
  return out;
}

RSocketConnection::RSocketConnection
(const ObjectCreationInfo& oi,
 const Par& par)
  : StdIdMember(oi.objectId),
    socket_rep(std::move(par.socket_rep))
{
  assert(socket_rep);
}

DEFINE_AXIS(
  ServerConnectionFactoryAxis,
  {}, {}
);

}
