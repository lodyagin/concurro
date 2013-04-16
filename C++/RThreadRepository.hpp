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
::delete_object(RThread<Thread>* thread, bool freeMemory)
{
  thread->stop();
  thread->is_terminated().wait();
  Parent::delete_object(thread, freeMemory);
}

template<class Thread, 
			template<class...> class Container,
			class ThreadId>
void RThreadRepository<Thread, Container, ThreadId>
//
::delete_object_by_id (ThreadId id, bool freeMemory)
{
  RThreadBase* th = this->get_object_by_id (id);
  if (th) 
  {
    th->stop ();
	 th->is_terminated().wait();
    Parent::delete_object_by_id (id, freeMemory);
  }
}

#endif
