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
 * @file An unified wrapper over different type of threads
 * (i.e., QThread, posix thread etc.).
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREAD_HPP_
#define CONCURRO_RTHREAD_HPP_

#include "RThread.h"
#include "RThreadRepository.h"

template<class Thread, class... Args>
Thread* RThread<std::thread>::create(Args&&... args)
{
  return dynamic_cast<Thread*>
	 (RThreadRepository<RThread<std::thread>>::instance()
	  . create_thread(typename Thread::Par(args...)));
}

#endif

