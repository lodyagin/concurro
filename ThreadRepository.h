#pragma once
#include "repository.h"
#include <list>
#include <algorithm>

template<class Thread, class Parameter>
class ThreadRepository :
  public Repository<Thread, Parameter>
{
public:
  ThreadRepository (int n)
    : Repository (n)
  {}

  //~ThreadRepository ();

  virtual void stop_subthreads ();
  virtual void wait_subthreads ();
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
    if (th) th->wait ();
  }
};

/*template<class Thread>
struct ThreadDestructor : std::unary_function<Thread*, void>
{
  void operator () (Thread* th)
  {
    delete th;
  }
};*/

template<class Thread, class Parameter>
void ThreadRepository<Thread,Parameter>::stop_subthreads ()
{
  std::for_each (
    objects->begin (),
    objects->end (),
    ThreadStopper<Thread> ()
    );
}

template<class Thread, class Parameter>
void ThreadRepository<Thread,Parameter>::wait_subthreads ()
{
  std::for_each (
    objects->begin (),
    objects->end (),
    ThreadWaiter<Thread> ()
    );
}

/*template<class Thread, class Parameter>
ThreadRepository<Thread,Parameter>::~ThreadRepository ()
{
  std::for_each (
    objects->begin (),
    objects->end (),
    ThreadDestructor<Thread> ()
    );
}*/