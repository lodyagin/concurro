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

#ifndef CONCURRO_RTHREADREPOSITORY_HPP_
#define CONCURRO_RTHREADREPOSITORY_HPP_

#include "RThreadRepository.h"
#include "RCheck.h"
#include <signal.h>

namespace curr {

template<class Thread, class T>
RThreadRepository<Thread, T>::RThreadRepository(int w)
  : Parent(typeid(RThreadRepository<Thread, T>).name(), 
           100 // the value is ignored for std::map
    ), wait_m(w)
{
  // Disable signals. See RSignalRepository
  sigset_t ss;
  rCheck(::sigfillset(&ss) == 0);
  // enable SIGINT, ^C is useful
  rCheck(::sigdelset(&ss, SIGINT) == 0);
  rCheck(::pthread_sigmask(SIG_SETMASK, &ss, NULL) == 0);
  
  this->log_params().get_object_by_id = false;
  this->complete_construction();
}


template<class Thread, class T>
void RThreadRepository<Thread, T>
//
::stop_subthreads ()
{
  this->for_each([](Thread& th)
  {
    if (th.is_running())
      th.stop();
  });
}

template<class Thread, class T>
void RThreadRepository<Thread, T>
//
::wait_subthreads ()
{
  this->for_each([this](Thread& th)
  {
    CURR_WAIT(th.is_terminated(), wait_m);
  });
}

template<class Thread, class T>
void RThreadRepository<Thread, T>
//
::cancel_subthreads ()
{
  this->for_each([](Thread& th)
  {
    th.cancel();
  });
}

template<class Thread, class T>
void RThreadRepository<Thread, T>
//
::delete_object(Thread* thread, bool freeMemory)
{
  assert (thread);
  const ThreadId id = fromString<ThreadId> 
    (thread->universal_id());
  delete_object_by_id(id, freeMemory);
}

template<class Thread, class T>
void RThreadRepository<Thread, T>
//
::delete_object_by_id (ThreadId id, bool freeMemory)
{
  RThreadBase* th = this->get_object_by_id (id);
  if (th) 
  {
    th->stop ();
    CURR_WAIT(th->is_terminal_state(), wait_m);
    Parent::delete_object_by_id (id, freeMemory);
  }
}

}
#endif
