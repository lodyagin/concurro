#ifndef __RMutex_H
#define __RMutex_H

#include <assert.h>
#include "SNotCopyable.h"

#include "Logging.h"

#include <boost/thread/mutex.hpp>
//plain versions
#define SLOCK(mutex)    RMutex::Lock   _lock  (mutex)
#define SUNLOCK(mutex)  RMutex::Unlock _unlock(mutex)

//debugging versions
#define SLOCKD(mutex)    LOG4CXX_DEBUG(Logging::Concurrency(), "SLOCK begin"); RMutex::Lock   _lock  (mutex,true,true);
#define SUNLOCKD(mutex)  LOG4CXX_DEBUG(Logging::Concurrency(), "SUNLOCK begin"); RMutex::Unlock _unlock(mutex,true);
//Logging before construction is to have proper filenames/linenumbers

class RMutex : public SNotCopyable
{
public:

  class Lock;
  class Unlock;

  RMutex();
  ~RMutex();

  void acquire();
  void release();

protected:
  boost::mutex mx;
};


class RMutex::Lock : public SNotCopyable
{
public:

  Lock
    (const RMutex &,
     bool lock = true 
     /*, bool wait = false*/);
  ~Lock();

  void acquire();
  void release();

private:

  RMutex & mutex;
  bool locked;
  bool debug;

};


class RMutex::Unlock : public SNotCopyable
{
public:

  Unlock( const RMutex & ,bool debug = false);
  ~Unlock();

private:

  RMutex & mutex;
  //bool debug;

};





// RMutex  ===========================================================

inline RMutex::RMutex()
{
  //InitializeCriticalSection(&cs);
}

inline RMutex::~RMutex()
{
  //DeleteCriticalSection(&cs);
}

inline void RMutex::acquire()
{
	mx.lock();
}

inline void RMutex::release()
{
	mx.unlock();
}

// RMutex::Lock  =====================================================

inline RMutex::Lock::Lock
    (const RMutex & m,
     bool lock 
     /*, bool wait*/
     )
 :
  mutex(const_cast<RMutex &>(m)),
				  locked(lock)//,debug(_debug)
{
  if ( lock ) {
	 LOG_DEBUG(Logger<LOG::Concurrency>, "Acquiring mutex");
  }

  /*if (wait)
	 mutex.wait ();
    else*/
  mutex.acquire();

  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
}

inline RMutex::Lock::~Lock()
{
  if ( locked ) { 
	 LOG_DEBUG(Logger<LOG::Concurrency>, "Releasing mutex");
  }
  mutex.release(); 
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
}

inline void RMutex::Lock::acquire()
{
  assert(!locked); // precondition
  LOG_DEBUG(Logger<LOG::Concurrency>, "Acquiring mutex");
  mutex.acquire();
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
  locked = true;
}

inline void RMutex::Lock::release()
{
  assert(locked); // precondition
  LOG_DEBUG(Logger<LOG::Concurrency>, "Releasing mutex");
  mutex.release();
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
  locked = false;
}


// RMutex::Unlock  ===================================================

inline RMutex::Unlock::Unlock( const RMutex & m , bool _debug ) :
				  mutex(const_cast<RMutex &>(m))//, debug(_debug)
{
  LOG_DEBUG(Logger<LOG::Concurrency>, "Releasing mutex");
  mutex.release();
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
}

inline RMutex::Unlock::~Unlock()
{
  LOG_DEBUG(Logger<LOG::Concurrency>, "Acquiring mutex");
  mutex.acquire();
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
}


#endif  
