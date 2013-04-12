// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREADREPOSITORY_HPP_
#define CONCURRO_RTHREADREPOSITORY_HPP_

#include "RThreadRepository.h"

template<class Thread, 
			template<class...> class Container, 
			class ThreadId>
void RThreadRepository<Thread, Container, ThreadId>
//
::stop_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadStopper<typename RepositoryMapType
	               <RThread<Thread>, ThreadId, Container>
                  ::Map::value_type
                  > ()
    );
}

template<class Thread, 
			template<class...> class Container, 
			class ThreadId>
void RThreadRepository<Thread, Container, ThreadId>
//
::wait_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadWaiter<typename RepositoryMapType
	               <RThread<Thread>, ThreadId, Container>
                  ::Map::value_type
                  > ()
    );
}

template<class Thread, 
			template<class...> class Container,
			class ThreadId>
void RThreadRepository<Thread, Container, ThreadId>
//
::delete_object_by_id (ThreadId id, bool freeMemory)
{
  RThreadBase* th = get_object_by_id (id);
  if (th) 
  {
    th->stop ();
	 RThreadBase::is_terminated().wait(*th);
    Parent::delete_object_by_id (id, freeMemory);
  }
}

#endif
