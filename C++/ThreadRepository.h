#pragma once
#include "Repository.h"
#include <list>
#include <algorithm>

template<class Thread, class Parameter>
class ThreadRepository :
  public Repository<Thread, Parameter>
{
public:
  typedef typename Repository<Thread,Parameter>::ObjectId ObjectId;

  ThreadRepository (int n)
    : Repository<Thread,Parameter> (n)
  {}

  virtual void stop_subthreads ();
  virtual void wait_subthreads ();

  // Overrides
  void delete_object_by_id 
    (ObjectId id, bool freeMemory);

  // Overrides
  Thread* replace_object 
    (ObjectId id, 
     const Parameter& param,
     bool freeMemory
     )
  {
    THROW_EXCEPTION
      (SException, 
       L"replace_object is not realised for threads."
       );
  }
};

template<class Thread>
struct ThreadStopper : std::unary_function<Thread*, void>
{
  void operator () (Thread* th)
  {
    if (th && th->is_running ()) th->stop ();
  }
};

template<class Thread>
struct ThreadWaiter : std::unary_function<Thread*, void>
{
  void operator () (Thread* th)
  {
#ifdef EVENT_IMPLEMENTED
    if (th) th->wait ();
#endif
  }
};

template<class Thread, class Parameter>
void ThreadRepository<Thread,Parameter>::stop_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadStopper<Thread> ()
    );
}

template<class Thread, class Parameter>
void ThreadRepository<Thread,Parameter>::wait_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadWaiter<Thread> ()
    );
}

template<class Thread, class Parameter>
void ThreadRepository<Thread,Parameter>::
  delete_object_by_id (typename Repository<Thread,Parameter>::ObjectId id, bool freeMemory)
{
  Thread* th = get_object_by_id (id);
  if (th) 
  {
    th->stop ();
#ifdef EVENT_IMPLEMENTED
    th->wait ();
#endif
    Repository<Thread,Parameter>::delete_object_by_id (id, freeMemory);
  }
}

