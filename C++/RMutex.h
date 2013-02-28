// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef __RMutex_H
#define __RMutex_H

#define MUTEX_BUG

#include <assert.h>
#include "SNotCopyable.h"

#include "Logging.h"
#include <log4cxx/spi/location/locationinfo.h>

#include <boost/thread/mutex.hpp>
//plain versions
#define RLOCK(mutex) \
  RMutex::Lock _lock(mutex, true, LOG4CXX_LOCATION)

#define RUNLOCK(mutex) \
  RMutex::Unlock _unlock(mutex, LOG4CXX_LOCATION)

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

/// Add an RAII semantics to RMutex - do acquire in the
/// constructor and release in the destructor
class RMutex::Lock : public SNotCopyable
{
public:

  Lock(const RMutex &,
		 bool lock = true,
		 const log4cxx::spi::LocationInfo& debug_location = 
	      LOG4CXX_LOCATION);
  ~Lock();

  void acquire();
  void release();

protected:
  const log4cxx::spi::LocationInfo location;

private:
  RMutex & mutex;
  bool locked;
};

/// Like RMutex::Lock but do release in the constructor
/// and acquire in the descrutctor
class RMutex::Unlock : public SNotCopyable
{
public:

  Unlock(const RMutex &,
			const log4cxx::spi::LocationInfo& debug_location = 
	        LOG4CXX_LOCATION);
  ~Unlock();

protected:
  const log4cxx::spi::LocationInfo location;

private:
  RMutex & mutex;
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
 bool lock,
 const log4cxx::spi::LocationInfo& debug_location
) : mutex(const_cast<RMutex &>(m)), locked(lock),
  location (debug_location)
{
  if ( lock ) {
	 LOG_DEBUG_LOC(Logger<LOG::Concurrency>, 
						"Acquiring mutex",
						location);

	 /*if (wait)
		mutex.wait ();
		else*/
	 mutex.acquire();

	 LOG_DEBUG_LOC(Logger<LOG::Concurrency>, "Success",
				  location);
  }
}

inline RMutex::Lock::~Lock()
{
  if ( locked ) { 
	 LOG_DEBUG_LOC(Logger<LOG::Concurrency>, 
						"Releasing mutex",
						location);
	 mutex.release(); 
	 LOG_DEBUG_LOC(Logger<LOG::Concurrency>, "Success",
						location);
  }
}

inline void RMutex::Lock::acquire()
{
  assert(!locked); // precondition
  LOG_DEBUG(Logger<LOG::Concurrency>, 
				"Acquiring mutex by RMutex::Lock::acquire()");
  mutex.acquire();
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
  locked = true;
}

inline void RMutex::Lock::release()
{
  assert(locked); // precondition
  LOG_DEBUG(Logger<LOG::Concurrency>, 
				"Releasing mutex by RMutex::Lock::release()");
  mutex.release();
  LOG_DEBUG(Logger<LOG::Concurrency>, "Success");
  locked = false;
}


// RMutex::Unlock  ===================================================

inline RMutex::Unlock::Unlock
(const RMutex & m, 
 const log4cxx::spi::LocationInfo& debug_location
  ) : mutex(const_cast<RMutex &>(m)),
  location(debug_location) 
{
  LOG_DEBUG_LOC(Logger<LOG::Concurrency>, "Releasing mutex",
					 location);
  mutex.release();
  LOG_DEBUG_LOC(Logger<LOG::Concurrency>, "Success", 
					 location);
}

inline RMutex::Unlock::~Unlock()
{
  LOG_DEBUG_LOC(Logger<LOG::Concurrency>, "Acquiring mutex",
					 location);
  mutex.acquire();
  LOG_DEBUG_LOC(Logger<LOG::Concurrency>, "Success",
					 location);
}

#endif  
