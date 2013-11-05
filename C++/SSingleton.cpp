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

#include "SSingleton.hpp"
#include "StateMapRepository.h"
#include "RThreadRepository.h"
#include "RState.hpp"
#include "types/ext_constr.h"

namespace curr {

DEFINE_AXIS(
  SingletonAxis,
  {},
  { {"preinc_exist_several", "exist_one"} // failback
  });

SAutoSingletonRepository()
//
::~SAutoSingletonRepository()
{
  SAutoSingletonBase* ptr;

  while (ases.pop(ptr))
    delete ptr;
}

void SAutoSingletonRegistry
//
::reg(SAutoSingletonBase* ptr)
{
  ases.push(ptr);
}

externally_constructed<SAutoSingletonRegistry> auto_reg;

int SAutoSingletonBase::nifty_counter;

SAutoSingletonBase::Init::Init()
{
  if (nifty_counter++ == 0)
  {
    new (&auto_reg.m) SAutoSingletonRegistry();
  }
}

ios_base::Init::~Init()
{
  if (--nifty_counter == 0)
  {
    auto_reg.m.~SAutoSingletonRegistry();
  }
}


}
