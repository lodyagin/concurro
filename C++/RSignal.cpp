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
#include "RSignal.h"

namespace curr {

RSignalBase* RSignalBase::Par
::create_derivation(const ObjectCreationInfo& oi) const
{
  switch(par.action) {
  case RSignalAction::Ignore:
	 return new RIgnoredSignal(oi, *this);
  case RSignalAction::Process:
	 return new RSignalHandler(oi, *this);
  default:
	 THROW_NOT_IMPLEMENTED;
  }
}

RSignalBase::RSignalBase
  (const ObjectCreationInfo& oi, const Par& par)
	 : signum(par.signum), action(par.action)
{}

void RSignalRepository::tune_signal
  (int signum, RSignalAction action)
{
  auto* sig = get_object_by_id(signum);
  if (!sig) {
	 sig = create_object(RSignalBase::Par(signum, action));
  }
  else {
	 replace_object(id, RSignalBase::Par(signum, action),
						 true);
  }
}

}
