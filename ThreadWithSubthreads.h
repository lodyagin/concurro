#pragma once
#include "sthread.h"
#include "ThreadRepository.h"

// SubthreadParameter is a parameter
// for subthread creation

template<class Subthread, class SubthreadParameter>
class ThreadWithSubthreads 
  : public SThread,
    public ThreadRepository<Subthread, SubthreadParameter>
{
public:

  // parameter to constructor
  typedef unsigned ConstrPar;

  ~ThreadWithSubthreads () {}

  static ThreadWithSubthreads& current ()
  { 
    return reinterpret_cast<ThreadWithSubthreads&>
      (SThread::current ());
  }

  virtual Subthread* create_subthread
    (const SubthreadParameter& pars)
  {
    return create_object (pars);
  }

  // Overrides
  // Wait termination of all subthreads
  void wait () 
  { 
    wait_subthreads(); 
    SThread::wait (); 
  }

  // Overrides
  // Request stop all subthreads and this thread
  void stop () 
  { 
    stop_subthreads(); 
    SThread::stop (); 
  }

protected:
  ThreadWithSubthreads (unsigned nSubthreadsMax) 
    : SThread (),
      ThreadRepository
        <Subthread, SubthreadParameter> 
          (nSubthreadsMax)
  {}
};
