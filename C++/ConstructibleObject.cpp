/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

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
#if 0
  { "in_construction", "complete_construction" },
  { { "in_construction", "complete_construction" } }
#else
  { "preinc_exist_one", "exist_one" },
  { { "preinc_exist_one", "exist_one" } }
#endif
  );

ConstructibleObject::ConstructibleObject()
  : RObjectWithEvents<ConstructibleAxis>
      (preinc_exist_oneFun()),
    CONSTRUCT_EVENT(exist_one, false)
{
}

ConstructibleObject::ConstructibleObject(
  const is_exist_one_event_t& cl_ev
)
  : RObjectWithEvents<ConstructibleAxis>
      (preinc_exist_oneFun()),
    is_exist_one_event(cl_ev)
{
}

void ConstructibleObject::complete_construction()
{
  move_to(*this, exist_oneFun());
}

}




