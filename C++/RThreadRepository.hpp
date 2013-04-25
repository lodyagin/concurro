// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREADREPOSITORY_HPP_
#define CONCURRO_RTHREADREPOSITORY_HPP_

#include "RThreadRepository.h"
#include <signal.h>

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
