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

template<class Val>
struct ThreadCanceller
  : std::unary_function<Val, void>
{
  void operator () (Val th)
  {
    if (th) th->cancel();
  }
};

template<class Key, class Val>
  struct ThreadCanceller<std::pair<Key, Val>>
  : std::unary_function<std::pair<Key, Val>&, void>
{
  void operator () (std::pair<Key, Val>& p)
  {
    if (p.second) 
      p.second->cancel();
  }
};


template<class Thread>
RThreadRepository<Thread>::RThreadRepository()
  : Parent(typeid(RThreadRepository<Thread>).name(), 
           100 // the value is ignored for std::map
    ) 
{
  // Disable signals. See RSignalRepository
  sigset_t ss;
  rCheck(::sigfillset(&ss) == 0);
  // enable SIGINT, ^C is useful
  rCheck(::sigdelset(&ss, SIGINT) == 0);
  rCheck(::pthread_sigmask(SIG_SETMASK, &ss, NULL) == 0);
  
  RepositoryBase<
    Thread, typename Thread::Par,
    std::map, typename Thread::Id
    >
    ::log_params.get_object_by_id = false;
}


template<class Thread>
void RThreadRepository<Thread>
//
::stop_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadStopper<typename RepositoryMapType
    <Thread, ThreadId, std::map>
    ::Map::value_type
    > ()
    );
}

template<class Thread>
void RThreadRepository<Thread>
//
::wait_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadWaiter<typename RepositoryMapType
    <Thread, ThreadId, std::map>
    ::Map::value_type
    > ()
    );
}

template<class Thread>
void RThreadRepository<Thread>
//
::cancel_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadCanceller<typename RepositoryMapType
    <Thread, ThreadId, std::map>
    ::Map::value_type
    > ()
    );
}

template<class Thread>
void RThreadRepository<Thread>
//
::delete_object(Thread* thread, bool freeMemory)
{
  assert (thread);
  const ThreadId id = fromString<ThreadId> 
    (thread->universal_id());
  delete_object_by_id(id, freeMemory);
}

template<class Thread>
void RThreadRepository<Thread>
//
::delete_object_by_id (ThreadId id, bool freeMemory)
{
  RThreadBase* th = this->get_object_by_id (id);
  if (th) 
  {
    th->stop ();
    th->is_terminal_state().wait();
    Parent::delete_object_by_id (id, freeMemory);
  }
}

}
#endif
