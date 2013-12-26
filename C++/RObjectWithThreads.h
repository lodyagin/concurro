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
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_ROBJECTWITHTHREADS_H_
#define CONCURRO_ROBJECTWITHTHREADS_H_

#include "RThread.h"
#include "ConstructibleObject.h"
#include <list>
#include <queue>
#include <functional>

namespace curr {

/**
 * @addtogroup threads
 * @{
 */

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
class RObjectWithThreads 
  : public virtual ConstructibleObject
{
public:
  using Parent = ConstructibleObject;
  using ThreadPar = ThreadOfObjectPar<Object>;

  //! Create the object and remember thread initialization
  //! pars. After the state will be changed to
  //! complete_construction for each parameter a thread
  //! will be created and started in
  //! RThreadRepository<RThread<std::thread>>
  //! The object takes ownership of all ThreadPar-s.
  RObjectWithThreads(std::initializer_list<ThreadPar*>);

  //! A deleted copy constructor.
  RObjectWithThreads(const RObjectWithThreads&) = delete;

  virtual ~RObjectWithThreads();

  //! A deleted assignment.
  RObjectWithThreads& operator=
    (const RObjectWithThreads&) = delete;

  void complete_construction() override
  {
    if (dynamic_cast<Object*>(this))
      Parent::complete_construction();
  }

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

template<class Object>
class ObjectThread : public StdThread
{
public:
  struct Par : public ThreadOfObjectPar<Object>
  {
    Par(const std::string& pretty_name)
      : ThreadOfObjectPar<Object>(pretty_name)
    {}
    
    //PAR_DEFAULT_OVERRIDE(RThreadBase, 
    //                     ObjectThread<Object>);
  };

  ~ObjectThread() { destroy(); }

protected:
  ObjectThread
    (const ObjectCreationInfo& oi, 
     const Par& par)
  : StdThread(oi, par),
    object(par.object)
  {
    assert(object);
  }

  Object* object;
};

template<class Object>
class ObjectFunThread : public ObjectThread<Object>
{
public:
  struct Par : ObjectThread<Object>::Par
  {
    std::function<void(Object&)> fun;

    template<class Fun>
    Par(const std::string& name, Fun f) 
      : ObjectThread<Object>::Par(name),
        fun(f) 
    {}

    PAR_DEFAULT_OVERRIDE(
      StdThread, ObjectFunThread<Object>);
  };

protected:
  std::function<void(Object&)> fun;

  ObjectFunThread
    (const ObjectCreationInfo& oi, 
     const typename ObjectThread<Object>::Par& par
     )
    : ObjectThread<Object>(oi, par),
      fun(dynamic_cast<const Par&>(par).fun)
  {}

  void run() override
  {
    move_to(*this, RThreadBase::workingState);
    assert(this->object);
    fun(*this->object);
  }
};

//! @}

}
#endif


