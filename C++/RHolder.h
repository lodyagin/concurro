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

#ifndef CONCURRO_RHOLDER_H_
#define CONCURRO_RHOLDER_H_

#include "RObjectWithStates.h"

namespace curr {

enum class HolderType { 
  Singular, //<! a view of one object
  Plural    //<! a view of an array of objects
};

DECLARE_AXIS(HolderAxis, StateAxis);

/**
 * It is a "view" of an object in a repository.
 */
class RHolder : public RObjectWithStates<HolderAxis>
{
  RHolder(const RHolder&);
  RHolder(RHolder&&);
  ~RHolder();

  RHolder& operator=(const RHolder&);
  RHolder& operator=(RHolder&&);
};

}

#endif

