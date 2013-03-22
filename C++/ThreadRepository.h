#ifndef CONCURRO_THREADREPOSITORY_H_
#define CONCURRO_THREADREPOSITORY_H_

#include "Repository.h"
#include "RThread.h"
#include <list>
#include <algorithm>
#include <vector>

template<class Thread, class Map, class ObjectId>
class ThreadRepository :
  public Repository<
#if 0
	 RThreadBase, 
	 RThreadBase::Par, 
#else
	 Thread, typename Thread::Par,
#endif
	 Map, ObjectId
	 >
{
public:
  typedef Repository
	 <Thread, typename Thread::Par, Map, ObjectId> Parent;

  ThreadRepository
	 (const std::string& repository_name, 
	  size_t reserved_size) 
	 : Parent(repository_name, reserved_size) {}

  virtual void stop_subthreads ();
  virtual void wait_subthreads ();

  // Overrides
  void delete_object_by_id 
    (ObjectId id, bool freeMemory);

  // Overrides
  RThreadBase* replace_object 
    (ObjectId id, 
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

struct ThreadStopper 
 : std::unary_function<RThreadBase*, void>
{
  void operator () (RThreadBase* th)
  {
    if (th && th->is_running ()) th->stop ();
  }
};

struct ThreadWaiter 
 : std::unary_function<RThreadBase*, void>
{
  void operator () (RThreadBase* th)
  {
    if (th) th->wait ();
  }
};

template<class Thread, class Map, class ObjectId>
void ThreadRepository<Thread, Map, ObjectId>::stop_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadStopper ()
    );
}

template<class Thread, class Map, class ObjectId>
void ThreadRepository<Thread, Map, ObjectId>::wait_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadWaiter ()
    );
}

template<class Thread, class Map, class ObjectId>
void ThreadRepository<Thread, Map, ObjectId>
//
::delete_object_by_id (ObjectId id, bool freeMemory)
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

