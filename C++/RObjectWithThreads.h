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

#ifndef CONCURRO_ROBJECTWITHTHREADS_H_
#define CONCURRO_ROBJECTWITHTHREADS_H_

#include "RThread.h"
#include <list>
#include <queue>

namespace curr {

/**
 * @addtogroup threads
 * @{
 */

DECLARE_AXIS(ConstructibleAxis, StateAxis);

//! A technical object. Must be moved to the
//! "complete_construction" state in the last derivative,
//! for example, when threads are ready to start (see
//! RObjectWithThreads). 
class RConstructibleObject
  : public RObjectWithEvents<ConstructibleAxis>
{
  DECLARE_EVENT(ConstructibleAxis, complete_construction);

public:
  DECLARE_STATES(ConstructibleAxis, ConstructibleState);
  DECLARE_STATE_CONST(ConstructibleState, in_construction);
  DECLARE_STATE_CONST(ConstructibleState, 
                      complete_construction);

  RConstructibleObject();
};

template<class Object>
struct ThreadOfObjectPar 
  : public RThread<std::thread>::Par
{
public:
  ThreadOfObjectPar(const std::string& pretty_name)
  { 
    thread_name = pretty_name; 
  }

  Object* object;
};

/**
 * An object which owns one or several threads.
 */
template<class Object>
class RObjectWithThreads : public RConstructibleObject
{
public:
  using ThreadPar = ThreadOfObjectPar<Object>;

  //! Create the object and remember thread initialization
  //! pars. After the state will be changed to
  //! complete_construction for each parameter a thread
  //! will be created and started in
  //! RThreadRepository<RThread<std::thread>>
  RObjectWithThreads(std::initializer_list<ThreadPar*>);

  //! A deleted copy constructor.
  RObjectWithThreads(const RObjectWithThreads&) = delete;

  virtual ~RObjectWithThreads();

  //! A deleted assignment.
  RObjectWithThreads& operator=
    (const RObjectWithThreads&) = delete;

protected:
  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override;

  //! A real work of a destructor is here. All descendants
  //! must call it in the destructor. It waits a
  //! termination of all threads. (Must be called from the
  //! most overrided destructor to prevent destroying
  //! vtable.
  void destroy();

  bool destructor_delegate_is_called;
  std::queue< std::unique_ptr<ThreadPar> > threads_pars;
  std::list<RThreadBase*> threads;
  std::list<CompoundEvent> threads_terminals;
};

//! @}

}
#endif


