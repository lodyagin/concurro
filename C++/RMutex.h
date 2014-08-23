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

#include <assert.h>
#include "SNotCopyable.h"
#ifdef USE_LOG4CXX
#include <log4cxx/spi/location/locationinfo.h>
#endif
#ifndef _WIN32
#include <boost/thread/recursive_mutex.hpp>
#endif

namespace curr {

/**
 * @defgroup synchronization
 * Sunchronization.
 * @{
 */

#ifdef USE_LOG4CXX

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

#else

#define MUTEX_ACQUIRE(mutex) \
  do { (mutex).acquire(); } while(0)

#define MUTEX_RELEASE(mutex) \
  do { (mutex).release(); } while(0)

/// RAII version of MUTEX_ACQUIRE
#define RLOCK(mutex) \
  curr::RMutex::Lock _lock(mutex)

/// RAII version of MUTEX_RELEASE (will acquire the mutex back on destruction)
#define RUNLOCK(mutex) \
  curr::RMutex::Unlock _unlock(mutex)

#endif

class RMutex : public SNotCopyable
{
  friend class RMutexArray;
public:

  class Lock;
  class Unlock;

  explicit RMutex(const std::string& the_name);
  ~RMutex();

  void acquire(
#ifdef USE_LOG4CXX
    const log4cxx::spi::LocationInfo& debug_location
#endif
  );
  void release(
#ifdef USE_LOG4CXX
    const log4cxx::spi::LocationInfo& debug_location
#endif
  );
  bool is_locked();

  const std::string get_name () const { return name; }

  void set_name (const std::string new_name) 
  {
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

  Lock(
      const RMutex &
#ifdef USE_LOG4CXX
    , const log4cxx::spi::LocationInfo& debug_location = 
        LOG4CXX_LOCATION
#endif
  );
  ~Lock();

  void acquire();
  void release();

protected:
#ifdef USE_LOG4CXX
  const log4cxx::spi::LocationInfo location;
#endif

private:
  RMutex & mutex;
};

/// Like RMutex::Lock but do release in the constructor
/// and acquire in the descrutctor
class RMutex::Unlock : public SNotCopyable
{
public:

  Unlock(const RMutex &/*, 
      const log4cxx::spi::LocationInfo& debug_location = 
          LOG4CXX_LOCATION*/);
  ~Unlock();

protected:
//  const log4cxx::spi::LocationInfo location;

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

inline void RMutex::acquire(
#ifdef USE_LOG4CXX
  const log4cxx::spi::LocationInfo& debug_location
#endif
)
{
  mx.lock();
}

inline void RMutex::release(
#ifdef USE_LOG4CXX
  const log4cxx::spi::LocationInfo& debug_location
#endif
)
{
  mx.unlock();
}

inline bool RMutex::is_locked()
{
  return !mx.try_lock();
}

// RMutex::Lock  =====================================================

inline RMutex::Lock::Lock 
(const RMutex & m
#ifdef USE_LOG4CXX
,const log4cxx::spi::LocationInfo& debug_location
#endif
) 
:   
#ifdef USE_LOG4CXX
    location (debug_location),
#endif
    mutex(const_cast<RMutex &>(m))
{
   mutex.acquire(
#ifdef USE_LOG4CXX
     location
#endif
   );
}

inline RMutex::Lock::~Lock()
{
   mutex.release(
#ifdef USE_LOG4CXX
     location
#endif
   ); 
}

inline void RMutex::Lock::acquire()
{
  mutex.acquire(
#ifdef USE_LOG4CXX
    location
#endif
  );
}

inline void RMutex::Lock::release()
{
  mutex.release(
#ifdef USE_LOG4CXX
    location
#endif
  );
}


// RMutex::Unlock  ========================================

inline RMutex::Unlock::Unlock(
    const RMutex & m
#ifdef USE_LOG4CXX
  , const log4cxx::spi::LocationInfo& debug_location
#endif
  ) 
: 
#ifdef USE_LOG4CXX
  location(debug_location),
#endif
  mutex(const_cast<RMutex &>(m))
{
  mutex.release(
#ifdef USE_LOG4CXX
    location
#endif
  );
}

inline RMutex::Unlock::~Unlock()
{
  mutex.acquire(
#ifdef USE_LOG4CXX
    location
#endif
  );
}

//! @}

}
#endif  

