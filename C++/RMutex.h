// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef __RMutex_H
#define __RMutex_H

#define MUTEX_BUG

#include <assert.h>
#include "SNotCopyable.h"

#include "Logging.h"
#include <log4cxx/spi/location/locationinfo.h>
//--------------------------------------------------------------

#ifdef WIN
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//plain versions
#define SLOCK(mutex)    SMutex::Lock   _lock  (mutex)
#define SUNLOCK(mutex)  SMutex::Unlock _unlock(mutex)

//debugging versions
#define SLOCKD(mutex)    LOG4CXX_DEBUG(Logging::Concurrency(), "SLOCK begin"); SMutex::Lock   _lock  (mutex,true,true);
#define SUNLOCKD(mutex)  LOG4CXX_DEBUG(Logging::Concurrency(), "SUNLOCK begin"); SMutex::Unlock _unlock(mutex,true);
//Logging before construction is to have proper filenames/linenumbers

class SMutex : public SNotCopyable
{
public:

  class Lock;
  class Unlock;

  SMutex();
  ~SMutex();

  void acquare();
  void release();

protected:
  CRITICAL_SECTION cs;
};

class SMutex::Lock : public SNotCopyable
{
public:
  Lock
    (const SMutex &,
     bool lock = true,
     bool debug = false/*,
     bool wait = false*/);
  ~Lock();

  void acquare();
  void release();

private:
  SMutex & mutex;
  bool locked;
  bool debug;
};


class SMutex::Unlock : public SNotCopyable
{
public:
  Unlock( const SMutex & ,bool debug = false);
  ~Unlock();
private:
  SMutex & mutex;
  bool debug;
};


// base for synchronized classes
class SSynchronized : public SMutex
{
public:
  SSynchronized() {}
};


template<class T>
class SMTValue : public SSynchronized
{
public:
  explicit SMTValue( const T & init ) : value(init) {}
  ~SMTValue() {}

  T get();
  void set( const T & );

protected:
  T value;
};

typedef SMTValue<int>  SMTInt;
typedef SMTValue<bool> SMTBool;


class SMTCounter : public SMTInt
{
public:

  typedef SMTInt Parent;

  explicit SMTCounter( int init = 0 ) : Parent(init) {}

  int operator ++ ();
  int operator ++ ( int );

};


// SMutex  ===========================================================

inline SMutex::SMutex()
{
  InitializeCriticalSection(&cs);
}

inline SMutex::~SMutex()
{
  DeleteCriticalSection(&cs);
}

inline void SMutex::acquare()
{
	EnterCriticalSection(&cs);
}

inline void SMutex::release()
{
	LeaveCriticalSection(&cs);
}

// SMutex::Lock  =====================================================

inline SMutex::Lock::Lock
    (const SMutex & m,
     bool lock ,
     bool _debug/*,
     bool wait*/
     )
 :
  mutex(const_cast<SMutex &>(m)),
  locked(lock),debug(_debug)
{
	if ( lock ) {
		 if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Acquiring mutex"); }

		/*if (wait)
      mutex.wait ();
    else*/
      mutex.acquare();

		 if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Success"); }
	}
}

inline SMutex::Lock::~Lock()
{
	if ( locked ) {
		 if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Releasing mutex"); }
		mutex.release();
		 if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Success"); }
	}
}

inline void SMutex::Lock::acquare()
{
  assert(!locked); // precondition
   if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Acquiring mutex"); }
  mutex.acquare();
   if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Success"); }
  locked = true;
}

inline void SMutex::Lock::release()
{
  assert(locked); // precondition
   if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Releasing mutex"); }
  mutex.release();
   if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Success"); }
  locked = false;
}


// SMutex::Unlock  ===================================================

inline SMutex::Unlock::Unlock( const SMutex & m , bool _debug ) :
  mutex(const_cast<SMutex &>(m)), debug(_debug)
{
    if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Releasing mutex"); }
  mutex.release();
    if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Success"); }
}

inline SMutex::Unlock::~Unlock()
{
    if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Acquiring mutex"); }
  mutex.acquare();
    if (debug) { LOG4CXX_DEBUG(Logging::Concurrency(), "Success"); }
}


// SMTValue  =========================================================

template<class T>
T SMTValue<T>::get()
{
  SLOCK(*this);
  return value;
}

template<class T>
void  SMTValue<T>::set( const T & v )
{
  SLOCK(*this);
  value = v;
}


// SMTCounter  =======================================================

inline int SMTCounter::operator ++ ()
{
  SLOCK(*this);
  return ++value;
}

inline int SMTCounter::operator ++ ( int )
{
  SLOCK(*this);
  return value++;
}

#else
//--------------------------------------------------------------
#include <boost/thread/recursive_mutex.hpp>
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
  boost::recursive_mutex mx;
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
#endif
