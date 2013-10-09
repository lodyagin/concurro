/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RTHREADREPOSITORY_H_
#define CONCURRO_RTHREADREPOSITORY_H_

#include "Repository.h"
#include "RThread.h"
#include "SSingleton.h"
#include <list>
#include <algorithm>
#include <vector>

namespace curr {

/**
 * @addtogroup threads
 * @{
 */

class RThreadFactory
{
public:
  virtual ~RThreadFactory() {}

  //! It delegates the call to
  //! RThreadRepository::create_object
  virtual RThreadBase* create_thread
	 (const RThreadBase::Par&) = 0;

  //! It delegates the call to
  //! RThreadRepository::delete_object(thread, true)
  virtual void delete_thread
    (RThreadBase* thread, bool freeMemory = true) = 0;
};

template<class Thread>
class RThreadRepository final
  : public virtual RThreadFactory,
    public Repository<
      Thread,
      typename Thread::Par, 
      std::map, 
      typename Thread::Id
    >,
    public SAutoSingleton<RThreadRepository<Thread>>
{
public:
  typedef Repository <
      Thread,
      typename Thread::Par, 
      std::map, 
      typename Thread::Id
  > Parent;
  typedef typename Thread::Par Par;
  typedef typename Thread::Id ThreadId;
  typedef typename Parent::NoSuchId NoSuchId;
  typedef typename Parent::IdIsAlreadyUsed IdIsAlreadyUsed;

  RThreadRepository(int w = 1000);
  //! Init with the specified operation timeout in msecs
  RThreadRepository(const RThreadRepository&) = delete;
  RThreadRepository& operator=
    (const RThreadRepository&) = delete;

  //! Init with the specified operation timeout in msecs
  static void init(int w)
  {
    new RThreadRepository(w);
  }

  virtual void stop_subthreads ();
  virtual void wait_subthreads ();
  virtual void cancel_subthreads ();

  //! It overrides RThreadFactory::create_thread
  RThreadBase* create_thread (const RThreadBase::Par& par)
  {
    return Parent::create_object
      (dynamic_cast<const Par&>(par));
  }

  //! It overrides RThreadFactory::delete_thread
  void delete_thread
    (RThreadBase* thread, bool freeMemory = true)
  {
    delete_object(dynamic_cast<Thread*>(thread), 
                  freeMemory);
  }

  // Overrides
  void delete_object(Thread* thread, bool freeMemory);

  // Overrides
  void delete_object_by_id(ThreadId id, bool freeMemory);

  // Overrides
  Thread* replace_object(ThreadId id,const Par& param,
                         bool freeMemory)
  {
    THROW_EXCEPTION
      (SException,
       "replace_object is not implemented for threads.");
  }

protected:
  int wait_m;

private:
  typedef Logger<RThreadRepository<Thread>> log;
};

//! @}

}
#endif

