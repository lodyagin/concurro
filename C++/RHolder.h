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
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RHOLDER_H_
#define CONCURRO_RHOLDER_H_

#include <atomic>
#include "types/exception.h"
#include "RObjectWithStates.h"
#include "RState.h"
#include "REvent.h"

namespace curr {

//! @addtogroup exceptions
//! @{

struct HolderException : virtual std::exception {};

//! @}

//! @addtogroup repositories
//! @{

enum class HolderType { 
  Singular, //<! a view of one object
  Plural    //<! a view of an array of objects
};

//! A common base for repository & singleton holders
template<
  class T,

  template<class, int>
  class Guard = T::template guard_templ,

  class StatesI = typename T::states_interface,

  int wait_m = 1000
>
class HolderCmn
  : public StatesI,
    public RHolderBase
{
  template<class O, class>
  friend auto compare_and_move(
    O& obj, 
    const RState<typename O::State::axis>& from,
    const RState<typename O::State::axis>& to
  )-> enable_fun_if<std::is_base_of<RHolderBase, O>, bool>;

public:
  //! Make a read-only object call (see
  //! NReaders1WriterGuard) 
  const typename Guard<T,wait_m>::ReadPtr operator->() 
    const
  {
    return guarded.operator->();
  }

  //! Make a read/write object call (see
  //! NReaders1WriterGuard) 
  typename Guard<T, wait_m>::WritePtr operator->()
  {
    return guarded.operator->();
  }

  HolderCmn* operator&() = delete;

  CompoundEvent is_terminal_state() const override
  {
    return guarded.get()->is_terminal_state();
  }

protected:
  HolderCmn(T* t) : guarded(t) {}

  void state_changed(
    curr::StateAxis& ax,
    const curr::StateAxis& state_ax,
    curr::AbstractObjectWithStates* object,
    const curr::UniversalState& new_state
  ) override
  {
    guarded->state_changed(ax,state_ax, object, new_state);
    /*RObjectWithStates<HolderAxis>
      ::state_changed(ax, state_ax, object, new_state);*/
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
    throw ::types::exception<HolderException>(
      "Call to RHolder::create_event()"
    );
  }

  void update_events
    (StateAxis& ax, 
     TransitionId trans_id, 
     uint32_t to) override
  {
    throw ::types::exception<HolderException>(
      "Call to RHolder::update_events()"
    );
  }

  // The guarded object
  Guard<T, wait_m> guarded;
};

// TODO RObjectWithStatesInterface and without states
// specializations 
/**
 * It is a "view" of an object T in a repository.
 *
 * \tparam Obj is a (polymorphic) type of a repository
 * object, T is the same or a derivation of Obj
 */
template
<
  class T,
  class Obj = T,

  template<class, int>
  class Guard = T::template guard_templ,

  class StatesI = typename T::states_interface,

  int wait_m = 1000
>
class RHolder 
  : public HolderCmn<T, Guard, StatesI, wait_m>
{
  using Parent = HolderCmn<T, Guard, StatesI, wait_m>;

public:
  typedef typename Obj::Par Par;

  //! Create a new object in the repository Rep and create
  //! the first holder to it.
  //! An AutoRepository is selected based on the (Obj,Id)
  //! pair
  template<class Id>
  RHolder(const Par& par);

  //! Charge with the object with the specified id.
  //! An AutoRepository is selected based on the (Obj,Id)
  //! pair
  //! @throw Rep::NoSuchId
  template<class Id>
  RHolder(const Id& id);

#if 0
  //! Double holder to the same object
  RHolder(const RHolder&);

  //! Do not double holder to the object
  RHolder(RHolder&&);

  //! Remove the holder. When the last object holder is
  //! removed object will be frozen
  ~RHolder();

  //! Double holder to the same object
  RHolder& operator=(const RHolder&);

  //! Do not double holder to the object
  RHolder& operator=(RHolder&&);
#endif

};

#if 0
template
<
  class Obj, class Par, template<class...> class ObjMap, 
  class ObjId, class Holder, class Guard
>
class ProtectedRepositoryInterface
  : public AbstractRepositoryBase
{
public:
  virtual RHolder create_object
};

template< 
  class Obj, 
  class Par, 
  template<class...> class ObjMap, 
  class ObjId,
  class Holder,
  class Guard
>
class ProtectedRepository  
  : public Repository<Obj, Par, ObjMap, ObjId>
{
  ProtectedRepository
    (const std::string& repository_name,
     size_t initial_capacity);
};
#endif

//! @}

}

#endif


