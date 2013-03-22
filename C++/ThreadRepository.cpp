#include "ThreadRepository.h"

void ThreadRepository::stop_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadStopper ()
    );
}

void ThreadRepository::wait_subthreads ()
{
  std::for_each (
    this->objects->begin (),
    this->objects->end (),
    ThreadWaiter ()
    );
}

void ThreadRepository::
  delete_object_by_id (ObjectId id, bool freeMemory)
{
  RThreadBase* th = get_object_by_id (id);
  if (th) 
  {
    th->stop ();
    th->wait ();
    Parent::delete_object_by_id (id, freeMemory);
  }
}

