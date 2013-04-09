// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_THREADREPOSITORY_H_
#define CONCURRO_THREADREPOSITORY_H_

#include "Repository.h"
#include "RThread.h"
#include <list>
#include <algorithm>
#include <vector>

class ThreadFactory
{
public:
  virtual RThreadBase* create_thread
	 (const RThreadBase::Par&) = 0;
};

template<
  class Thread, 
  template<class...> class Container, 
  class ThreadId
>
class ThreadRepository 
: public Repository<
  RThread<Thread>, 
  typename RThread<Thread>::Par, 
  Container,
  ThreadId >,
  public ThreadFactory
{
public:
  typedef Repository<
    RThread<Thread>, 
    typename RThread<Thread>::Par, 
    Container,
    ThreadId> Parent;
  typedef typename RThread<Thread>::Par Par;

  ThreadRepository() 
	 : Parent(typeid(*this).name(), 100) {}

  virtual void stop_subthreads ();
  virtual void wait_subthreads ();

  RThreadBase* create_thread (const RThreadBase::Par& par)
  {
	 return Parent::create_object(par);
  }

  // Overrides
  void delete_object_by_id 
    (ThreadId id, bool freeMemory);

  // Overrides
  RThreadBase* replace_object 
    (ThreadId id, 
     const RThreadBase::Par& param,
     bool freeMemory
     )
  {
    THROW_EXCEPTION
      (SException, 
       "replace_object is not realised for threads."
       );
  }
};

template<class Val>
struct ThreadStopper
 : std::unary_function<Val, void>
{
  void operator () (Val th)
  {
    if (th && th->is_running ()) th->stop ();
  }
};

// std::pair version
template<class Key, class Val>
struct ThreadStopper<std::pair<Key, Val>>
 : std::unary_function<std::pair<Key, Val>&, void>
{
  void operator () (std::pair<Key, Val>& p)
  {
    if (p.second && p.second->is_running ()) 
		p.second->stop ();
  }
};

template<class Val>
struct ThreadWaiter
 : std::unary_function<Val, void>
{
  void operator () (Val th)
  {
    if (th) th->wait ();
  }
};

template<class Key, class Val>
struct ThreadWaiter<std::pair<Key, Val>>
 : std::unary_function<std::pair<Key, Val>&, void>
{
  void operator () (std::pair<Key, Val>& p)
  {
    if (p.second) 
		p.second->RThreadBase::wait ();
  }
};

#if 0
class AbstractThreadFactory
{
public:
  //! Create a thread and register it in the
  //! ThreadRepository
  virtual RThreadBase* create
	 (const RThreadBase::Par&) = 0;
};

template<class ThreadRepository>
class ThreadFactory
{
public:
  ThreadRepository *const thread_repository;

  ThreadFactory(ThreadRepository* tr)
	 : thread_repository(tr) { SCHECK(tr); }
  RThreadBase* create(const RThreadBase::Par&);
};
#else
#endif

#endif

