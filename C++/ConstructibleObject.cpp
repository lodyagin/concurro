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

#include "ConstructibleObject.h"
#include "RState.hpp"
#include "REvent.hpp"

namespace curr {

DEFINE_AXIS(
  ConstructibleAxis,
  { "in_construction", "complete_construction" },
  { { "in_construction", "complete_construction" }
  });

DEFINE_STATES(ConstructibleAxis);
DEFINE_STATE_CONST(ConstructibleObject, 
                   ConstructibleState, 
                   complete_construction);
DEFINE_STATE_CONST(ConstructibleObject, 
                   ConstructibleState, 
                   in_construction);

ConstructibleObject::ConstructibleObject()
  : RObjectWithEvents<ConstructibleAxis>
    (in_constructionState),
    CONSTRUCT_EVENT(complete_construction)
{
}

void ConstructibleObject::complete_construction()
{
  move_to(*this, S(complete_construction));
}

}




