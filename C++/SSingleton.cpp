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

namespace curr {

RMixedAxis<ExistenceAxis, ExistenceAxis>&
SAutoSingleton<RMixedAxis<ExistenceAxis, ExistenceAxis>> 
::instance()
{
  static std::once_flag of;
  static T* instance = nullptr;
  std::call_once(of, [](){ instance = new T(); });
  assert(instance);
  return *instance;
}

StateMapRepository& 
SAutoSingleton<StateMapRepository> 
::instance()
{
  static std::once_flag of;
  static T* instance = nullptr;
  std::call_once(of, [](){ instance = new T(); });
  assert(instance);
  return *instance;
}

RThreadRepository<RThread<std::thread>>& 
SAutoSingleton<RThreadRepository<RThread<std::thread>>> 
::instance()
{
  static std::once_flag of;
  static T* instance = nullptr;
  std::call_once(of, [](){ instance = new T(); });
  assert(instance);
  return *instance;
}

}
