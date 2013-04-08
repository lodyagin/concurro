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

template<class Thread, template<class...> class Container, class ThreadId>
class ThreadRepository 
: public Repository<
  RThread<Thread>, 
  typename RThread<Thread>::Par, 
  Container,
  ThreadId >,
  public SAutoSingleton<ThreadRepository<Thread, Container, ThreadId> >
{
public:
  typedef Repository<
    RThread<Thread>, 
    typename RThread<Thread>::Par, 
    Container,
    ThreadId> Parent;

  ThreadRepository() 
	 : Parent(typeid(*this).name(), 100) {}

  //! Create a thread and register it in the ThreadRepository
  template<class Th>
  static Th* create(Event* ext_terminated = 0);

  virtual void stop_subthreads ();
  virtual void wait_subthreads ();

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

#endif

