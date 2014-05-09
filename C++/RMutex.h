/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Boris Patochkin
 */

#ifndef __RMutex_H
#define __RMutex_H

//#include "Logging.h"
#include <assert.h>
#include "SNotCopyable.h"
//#include "Logging.h"
#include <log4cxx/spi/location/locationinfo.h>
//#include <atomic>
#ifndef _WIN32
#include <boost/thread/recursive_mutex.hpp>
#endif

namespace curr {

/**
 * @defgroup synchronization
 * Sunchronization.
 * @{
 */

#define MUTEX_ACQUIRE(mutex) \
  do { (mutex).acquire(LOG4CXX_LOCATION); } while(0)

#define MUTEX_RELEASE(mutex) \
  do { (mutex).release(LOG4CXX_LOCATION); } while(0)

/// RAII version of MUTEX_ACQUIRE
#define RLOCK(mutex) \
  curr::RMutex::Lock _lock(mutex, LOG4CXX_LOCATION)

/// RAII version of MUTEX_RELEASE (will acquire the mutex back on destruction)
#define RUNLOCK(mutex) \
  curr::RMutex::Unlock _unlock(mutex, LOG4CXX_LOCATION)

class RMutex : public SNotCopyable
{
  friend class RMutexArray;
public:

  class Lock;
  class Unlock;

  RMutex(const std::string& the_name);
  ~RMutex();

  void acquire(const log4cxx::spi::LocationInfo& debug_location);
  void release(const log4cxx::spi::LocationInfo& debug_location);
  bool is_locked();

  const std::string get_name () const { return name; }

  void set_name (const std::string new_name) {
   name = new_name;
  }

protected:
  RMutex() : name (":a mutex with an undefined name:") {}
  boost::recursive_mutex mx;

private:
//  typedef Logger<RMutex> log;
  
  std::string name;
};

class RMutexArray
{
public:
  RMutexArray(size_t sz, const std::string& initial_name);
  virtual ~RMutexArray () {}

  RMutex* data() { return &mutexes[0]; }

  const size_t size;
protected:
  RMutex* mutexes;
};

/// Add an RAII semantics to RMutex - do acquire in the
/// constructor and release in the destructor
class RMutex::Lock : public SNotCopyable
{
public:

  Lock(const RMutex &,
//       bool lock = true,
       const log4cxx::spi::LocationInfo& debug_location = 
       LOG4CXX_LOCATION);
  ~Lock();

  void acquire();
  void release();

protected:
  const log4cxx::spi::LocationInfo location;

private:
  RMutex & mutex;
  //std::atomic<bool> locked;
};

/// Like RMutex::Lock but do release in the constructor
/// and acquire in the descrutctor
class RMutex::Unlock : public SNotCopyable
{
public:

  Unlock(const RMutex &, 
//      bool, ///< dummy parameter to be compatible with RMutex::Lock::Lock
      const log4cxx::spi::LocationInfo& debug_location = 
          LOG4CXX_LOCATION);
  ~Unlock();

protected:
  const log4cxx::spi::LocationInfo location;

private:
  RMutex & mutex;
};





// RMutex  ===========================================================

/*inline RMutex::RMutex()
{
  //InitializeCriticalSection(&cs);
  }*/

inline RMutex::RMutex(const std::string& the_name) 
          : name (the_name)
{
  //InitializeCriticalSection(&cs);
}

inline RMutex::~RMutex()
{
  //DeleteCriticalSection(&cs);
}

inline void RMutex::acquire(const log4cxx::spi::LocationInfo& debug_location)
{
  /*  LOG_DEBUG_LOC(log, "try " << get_name() 
           << ".acquire {",
           debug_location);
  */
  mx.lock();
  /*  LOG_DEBUG_LOC(log, 
           "} " << get_name() << ".acquire done",
           debug_location);
  */
}

inline void RMutex::release(const log4cxx::spi::LocationInfo& debug_location)
{
  /*  LOG_DEBUG_LOC(log, "try " << get_name() 
           << ".release {",
           debug_location);
  */
  mx.unlock();
  /*  LOG_DEBUG_LOC(log, "} " << get_name() << ".release done",
           debug_location);
  */
}

inline bool RMutex::is_locked()
{
  return !mx.try_lock();
}

// RMutex::Lock  =====================================================

inline RMutex::Lock::Lock 
(const RMutex & m,
// bool lock,
 const log4cxx::spi::LocationInfo& debug_location
) 
:   location (debug_location),
    mutex(const_cast<RMutex &>(m))
{
//  if ( lock ) {
   /*if (wait)
    mutex.wait ();
    else*/
   mutex.acquire(location);
//  }
}

inline RMutex::Lock::~Lock()
{
//  if ( locked ) { 
   mutex.release(location); 
//  }
}

inline void RMutex::Lock::acquire()
{
//  assert(!locked); // precondition
  mutex.acquire(location);
//  locked = true;
}

inline void RMutex::Lock::release()
{
//  assert(locked); // precondition
  mutex.release(location);
//  locked = false;
}


// RMutex::Unlock  ===================================================

inline RMutex::Unlock::Unlock
(const RMutex & m, //bool,
 const log4cxx::spi::LocationInfo& debug_location
  ) 
: location(debug_location),
  mutex(const_cast<RMutex &>(m))
{
  mutex.release(location);
}

inline RMutex::Unlock::~Unlock()
{
  mutex.acquire(location);
}

//! @}

}
#endif  

