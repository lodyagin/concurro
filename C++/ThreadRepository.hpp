// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_THREADREPOSITORY_HPP_
#define CONCURRO_THREADREPOSITORY_HPP_

#include "ThreadRepository.h"

template<class Thread, template<class...> class Container, class ThreadId>
void ThreadRepository<Thread, Container, ThreadId>
//
::stop_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadStopper<typename RepositoryMapType
                  <Thread, ThreadId, Container>
                  ::Map::value_type
                  > ()
    );
}

template<class Thread, template<class...> class Container, class ThreadId>
void ThreadRepository<Thread, Container, ThreadId>
//
::wait_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadWaiter<typename RepositoryMapType
                  <Thread, ThreadId, Container>
                  ::Map::value_type
                  > ()
    );
}

template<class Thread, template<class...> class Container, class ThreadId>
void ThreadRepository<Thread, Container, ThreadId>
//
::delete_object_by_id (ThreadId id, bool freeMemory)
{
  RThreadBase* th = get_object_by_id (id);
  if (th) 
  {
    th->stop ();
    th->wait ();
    Parent::delete_object_by_id (id, freeMemory);
  }
}

#endif
