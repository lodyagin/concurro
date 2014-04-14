/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

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

#include "RObjectWithStates.hpp"
#include "RState.hpp"

namespace curr {

DEFINE_AXIS(
  MoveableAxis,
  {
//    "stable",
    "moving_from",
    "moving_to",
    "copying_from",
//    "copying_to",
    "moved_from",// state of the object after the content
                 // is moved from it
    "swapping"
  },
  {
//    { "stable", "moving_from" },
//    { "moving", "stable" },
    { "moving_from", "moved_from" },
//    { "stable", "copying" },
//    { "copying
  }
);

}
