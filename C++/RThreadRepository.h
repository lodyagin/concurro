// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREADREPOSITORY_H_
#define CONCURRO_RTHREADREPOSITORY_H_

#include "Repository.h"
#include "RThread.h"
#include <list>
#include <algorithm>
#include <vector>

class RThreadFactory
{
public:
  virtual ~RThreadFactory() {}

  //! It delegates the call to
  //! RThreadRepository::create_object
  virtual RThreadBase* create_thread
	 (const RThreadBase::Par&) = 0;

  //! It delegates the call to
  //! RThreadRepository::delete_object(thread, true)
  virtual void delete_thread
	 (RThreadBase* thread) = 0;
};

template<class Thread>
class RThreadRepository 
  : public Repository<
      Thread,
      typename Thread::Par, 
      std::map, 
      typename Thread::Id
  >,
  public RThreadFactory,
  public SAutoSingleton<RThreadRepository<Thread>>
{
public:
  typedef Repository<
      Thread,
      typename Thread::Par, 
      std::map, 
      typename Thread::Id
	 > Parent;
  typedef typename Thread::Par Par;
  typedef typename Thread::Id ThreadId;

  RThreadRepository(/*const std::string& name,
							 size_t initial_capacity*/) 
	 : Parent(typeid(RThreadRepository<Thread>).name(), 
				 100 // the value is ignored for std::map
		) {}


  virtual void stop_subthreads ();
  virtual void wait_subthreads ();

  //! It overrides RThreadFactory::create_thread
  RThreadBase* create_thread (const RThreadBase::Par& par)
  {
	  return Parent::create_object
		 (dynamic_cast<const Par&>(par));
  }

  //! It overrides RThreadFactory::delete_thread
  void delete_thread(RThreadBase* thread)
  {
	 delete_object(dynamic_cast<Thread*>(thread), true);
  }

  // Overrides
  void delete_object(Thread* thread, bool freeMemory);

  // Overrides
  void delete_object_by_id(ThreadId id, bool freeMemory);

  // Overrides
  Thread* replace_object(ThreadId id,const Par& param,
  		bool freeMemory)
  {
    THROW_EXCEPTION
    (SException,
    		"replace_object is not realised for threads.");
  }
};

template<class Obj>
struct ThreadStopper
 : std::unary_function<Obj, void>
{
  void operator () (Obj th)
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
    if (th) th->is_terminated().wait();
  }
};

template<class Key, class Val>
struct ThreadWaiter<std::pair<Key, Val>>
 : std::unary_function<std::pair<Key, Val>&, void>
{
  void operator () (std::pair<Key, Val>& p)
  {
    if (p.second) 
		p.second->is_terminated().wait();
  }
};

#endif

