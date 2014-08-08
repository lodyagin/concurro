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

#include "Existent.hpp"
#include "RState.hpp"
#include "RObjectWithStates.hpp"

namespace curr {

::types::constexpr_string existent_class_initial_state = 
  "not_exist";

DEFINE_AXIS(
  ExistenceAxis,
  {   "not_exist",
      "predec_exist_one",
      //"preinc_exist_one",
      //"exist_one",
      "moving_when_one",
      "predec_exist_several",
      "preinc_exist_several",
      "exist_several",
      "moving_when_several"
  },
  { // inc_existence()
    {"not_exist", "preinc_exist_one"},
    {"preinc_exist_one", "exist_one"},
    {"exist_one", "preinc_exist_several"},
    {"preinc_exist_several", "exist_several"},
    {"exist_several", "preinc_exist_several"},

    // dec_existence()
    {"exist_several", "predec_exist_several"},
    {"predec_exist_several", "exist_several"},
    {"predec_exist_several", "exist_one"},
    {"exist_one", "predec_exist_one"},
    {"predec_exist_one", "not_exist"},

    // moving constructor/assignment
    {"exist_one", "moving_when_one"},
    {"moving_when_one", "exist_one"},
    {"exist_several", "moving_when_several"},
    {"moving_when_several", "exist_several"},
  }
  );
}
