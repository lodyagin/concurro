// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREADREPOSITORY_HPP_
#define CONCURRO_RTHREADREPOSITORY_HPP_

#include "RThreadRepository.h"

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
	 th->is_terminated().wait();
    Parent::delete_object_by_id (id, freeMemory);
  }
}

#endif
