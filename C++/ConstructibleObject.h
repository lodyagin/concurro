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

#ifndef CONCURRO_CONSTRUCTIBLEOBJECT_H_
#define CONCURRO_CONSTRUCTIBLEOBJECT_H_

#include "RObjectWithStates.h"
#include "RState.h"
//#include "REvent.h"

namespace curr {

/**
 * @addtogroup basics
 * @{
 */

DECLARE_AXIS(ConstructibleAxis, StateAxis);

//! An inheritance from this object can prevent accessing
//! the object
//! from concurrent threads till it will be constructed
//! completely. It is used, for example, in
//! RObjectWithThreads. It must
//! be moved to the
//! "complete_construction" state in the last derivative.
class ConstructibleObject
#ifdef USE_EVENTS
  : public RObjectWithEvents<ConstructibleAxis>
#else
  : public RObjectWithStates<ConstructibleAxis>
#endif
{
#ifdef USE_EVENTS
  DECLARE_EVENT(ConstructibleAxis, complete_construction);
#endif

public:
  DECLARE_STATES(ConstructibleAxis, ConstructibleState);
  DECLARE_STATE_CONST(ConstructibleState, in_construction);
  DECLARE_STATE_CONST(ConstructibleState, 
                      complete_construction);

  ConstructibleObject();

  //! Report complete construction from a
  //! descendant. Change the state to
  //! complete_construction.
  virtual void complete_construction();
};

//! @}

}

#endif


