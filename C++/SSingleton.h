/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file 
 * Singletons. It always must be the first included
 * file from the concurro library.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_SSINGLETON_H_
#define CONCURRO_SSINGLETON_H_

#include <thread>
#include <exception>
#include "types/typeinfo.h"
#include "SCheck.h"
#include "Existent.h"
#include "ConstructibleObject.h"
#include "Event.h"
#include "RHolder.h"

namespace curr {

/**
 * @addtogroup exceptions
 * @{
 */

struct SingletonException : virtual std::exception {};

//! Exception: somebody tries to get an
//! SSingleton::instance() when no one is available
struct NotExistingSingleton : SingletonException {};

//! Exception: somebody tries to create more than one
//! SSingleton instance
struct MustBeSingleton : SingletonException {};

//! @}

/**
 * @defgroup singletons
 * Objects with counted number of instances.
 * @{
 */

DECLARE_AXIS(SingletonAxis, ExistenceAxis);

template<class T, int wait_m>
class SSingleton;

/**
 * A hook for check whether object is still singleton.
 */
template<class T, int wait_m>
class SingletonStateHook
{
  friend SSingleton<T, wait_m>;
public:
  //! Disable change to ExistenceAxis exist_several state
  //! @exception MustBeSingleton
  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state);
};

//! Holder for SSingleton
template<class T, int wait_m>
class SHolder
  : public HolderCmn<
      T, 
      T::template guard_templ, 
      event::interface<SingletonAxis>,
      wait_m
    >
{
  using Parent = HolderCmn<
      T, 
      T::template guard_templ, 
      event::interface<SingletonAxis>,
      wait_m
    >;
public:
  SHolder(T* t) : Parent(t) {}

};

/**
 * A base class for classes that can have only one instance
 * parametrised by the actual singleton class, use as:
 * class MyClass : public SSingleton<MyClass>
 *
 * @dot
 * digraph {
 *   start [shape = point]; 
 *   not_exist [shape = doublecircle];
 *   start -> not_exist;
 *   not_exist -> preinc_exist_one
 *     [label="ctr:0"];
 *   preinc_exist_one -> exist_one
 *     [label="ctr:1"];
 *   exist_one -> preinc_exist_several
 *     [label="ctr:0"];
 *   preinc_exist_several -> exist_one
 *     [label="ctr (throw MustBeSingleton)"];
 *   preinc_exist_several -> exist_several
 *     [label="ctr:1"];
 *   exist_several -> preinc_exist_several
 *     [label="ctr:0"];
 *   exist_several -> predec_exist_several
 *     [label="dtr:0"];
 *   predec_exist_several -> exist_several
 *     [label="dtr:1"];
 *   predec_exist_several -> exist_one
 *     [label="dtr:1"];
 *   exist_one -> predec_exist_one
 *     [label="dtr:0"];
 *   predec_exist_one -> not_exist
 *     [label="dtr:1"];
 *   exist_one -> preoccupied
 *     [label="occupy:0"];
 *   preoccupied -> occupied
 *     [label="occupy:1"];
 *   occupied -> preoccupied
 *     [label="occupy:0"];
 *   occupied -> postoccupied
 *     [label="yield:0"];
 *   postoccupied -> occupied
 *     [label="yield:1"];
 *   postoccupied -> exist_one
 *     [label="yield:1"];
 * }
 * @enddot
 *
 */
template<class T, int wait_m = 1000>
class SSingleton 
  : public Existent<T, SingletonStateHook<T, wait_m>>,
    public virtual event::interface<SingletonAxis>
{
  friend SingletonStateHook<T, wait_m>;

  typedef Existent<T, SingletonStateHook<T, wait_m>> 
    Parent;
//  typedef ConstructibleObject ObjParent;
  typedef SSingleton<T, wait_m> This;

  A_DECLARE_EVENT(
    SingletonAxis, ConstructibleAxis, occupied
  );

public:
  //! @cond
  DECLARE_STATES(SingletonAxis, State);
  DECLARE_STATE_CONST(State, preoccupied);
  DECLARE_STATE_CONST(State, occupied);
  DECLARE_STATE_CONST(State, postoccupied);
  //! @endcond

  static auto is_exist_one() 
  -> decltype(Parent::TheClass::instance()->is_exist_one())
  {
    static auto the_event = 
      Parent::TheClass::instance()->is_exist_one();
    return the_event;
  }

  struct NotExistingSingleton : curr::NotExistingSingleton
  {
    [[noreturn]] void raise() const;
  };

  struct MustBeSingleton : curr::MustBeSingleton
  {
    [[noreturn]] void raise() const;
  };

  //! A singleton guard 
  //! 1) doesn't allow destroy singleton in a middle of
  //! call;
  //! 2) doesn't allow make call on not-existing 
  //! (destructed) singleton
  template<class T1, int w1>
  class guard_templ
  {
  public:
    class Ptr
    {
    public:
      Ptr(guard_templ* g) : guard(g)
      {
        guard->occupy();
      }

      ~Ptr()
      {
        guard->yield();
      }

      T1* operator->()
      {
        return guard->obj;
      }

    private:
      guard_templ* guard;
    };

    using ReadPtr = Ptr;
    using WritePtr = Ptr;

    guard_templ() : guard_templ(nullptr) {}
    guard_templ(std::nullptr_t object) : obj(nullptr) {}
    guard_templ(T1* object) : obj(object) 
    {
      SCHECK(obj);
    }

    guard_templ(guard_templ&& g) noexcept 
      : obj(g.discharge()) 
    {}

    guard_templ& operator=(guard_templ&& g) noexcept
    {
      obj = g.discharge();
    }

    ~guard_templ() { discharge(); }

    guard_templ& charge(T1* object)
    {
      SCHECK(object);
      obj = object;
      return *this;
    }

    void swap(guard_templ& g) noexcept
    {
      std::swap(obj, g.obj);
    }

    T1* discharge() noexcept
    {
      T1* res = obj; // NB res can be nullptr
      obj = nullptr;
      return res;
    }

    T1* get()
    {
      return obj;
    }

    const T1* get() const
    {
      return obj;
    }

    operator bool() const
    {
      return obj;
    }

    Ptr operator->()
    {
      return Ptr(this);
    }
   
  protected:
    T1* obj;

    void occupy() const
    {
      obj->occupy();
    }

    void yield() const
    {
      obj->yield();
    }
  };

  using Holder = SHolder<T, wait_m>;

  //! One and only one class instance must be created with
  //! this function.
  //! @exception MustBeSingleton a copy of SSingleton<T,
  //! wait_m> already exists.
  SSingleton();

  //! A deleted copy constructor
  SSingleton(const SSingleton&) = delete;

  //! The move constructor doesn't change existence.
  SSingleton(SSingleton&& s) = delete;

  virtual ~SSingleton();

  //! A deleted assignment operator.
  SSingleton& operator=(const SSingleton&) = delete;

  //! The move assignment is impossible unless lvalue and
  //! rvalue is the same object.
  SSingleton& operator=(SSingleton&& s) = delete;

#if 0
  CompoundEvent is_terminal_state() const override
  {
    return is_terminal_state_event;
  }
#endif

  void complete_construction() override;

  //! Return a holder to the class instance. 
  //!
  //! @exception NotExistingSingleton If no
  //! class is crated with SSingleton() raise exception.
  static Holder instance()
  {
    return Holder(instance_intl());
  }

  //! It is the same as !state_is(*this, preinc_exist_one)
  bool isConstructed();

  void state_changed
    (StateAxis& ax, 
     const StateAxis& state_ax,     
     AbstractObjectWithStates* object,
     const UniversalState& new_state) override
  {
#if 1
    ax.state_changed(this, object, state_ax, new_state);
#else
    throw ::types::exception<SingletonException>(
      "SSingleton::state_changed() must not be called"
    );
#endif
  }

  std::atomic<uint32_t>& 
  current_state(const StateAxis& ax) override
  { 
    return ax.current_state(this);
  }

  const std::atomic<uint32_t>& 
  current_state(const StateAxis& ax) const override
  { 
    return ax.current_state(this);
  }

  CompoundEvent create_event(
    const StateAxis& ax, 
    const UniversalEvent& ue,
    bool logging = true
  ) const override
  {
    return ConstructibleObject::create_event
      (ax, ue, logging);
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    ax.update_events(this, trans_id, to);
  }

protected:
  //! Return instance without occupation check
  static T* instance_intl();

  //! Precondition to occupy()
  CompoundEvent is_occupiable_event = {
    Parent::TheClass::instance()->is_exist_one(), 
    is_occupied_event
  };
/*    
  //! Precondition to yield()
  CompoundEvent is_yieldable = {
    is_occupied_event
  };
*/
private:
  //! The actual singleton instance. It is updated from
  //! instance0 in instance() call.
  static T* _instance;

  mutable std::size_t occupy_count = 0;

 /* CompoundEvent is_terminal_state_event { 
    ConstructibleObject::is_exist_one()
  };*/

  void occupy() const;
  void yield() const;
};

//! It allows destroy SAutoSingleton - s.
class SAutoSingletonBase
{
public:
  virtual ~SAutoSingletonBase() {}
  
  virtual bool need_destruct() const = 0;

  //! We use the same method of initialization as std::cout
  class Init
  {
  public:
    Init();
    ~Init();
  protected:
    // TODO not atomic?
    static int nifty_counter;
  };
};

namespace {

//! It will help call SAutoSingleton destructors on exit.
SAutoSingletonBase::Init auto_reg_init_helper;

}

/**
 * Extends SSingleton to allow auto-construct it by the
 * RAutoSingleton::instance() call.
 * \tparam destruct Destroy on program exit
 * \tparam wait_m The default construction wait timeout.
 */
template<class T, bool destruct = true, int wait_m = 1000>
class SAutoSingleton 
  : public SSingleton<T, wait_m>,
    public virtual SAutoSingletonBase
{
  friend class SAutoSingletonRegistry;

public:
  //! Return the reference to the class instance. 
  //! Not safe in multithreading environment (need to
  //! redesign with RHolder).
  static typename SSingleton<T, wait_m>::Holder instance();

  bool need_destruct() const override
  {
    return destruct;
  }

protected:
  //! Create the object if doesn't exist
  static void construct_once();
};

//! @}

}
#endif
