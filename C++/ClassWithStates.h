/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
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

#ifndef CONCURRO_CLASSWITHSTATES_H_
#define CONCURRO_CLASSWITHSTATES_H_

#include "ObjectWithStatesInterface.h"

namespace curr {

//! @addtogroup states
//! @{

template <
  class ClassAxis, 
  const char* initial,
  class StateHook
>
class ClassWithStates;

template<class ClassAxis, const char* initial>
class EmptyStateHook
{
public:
  void operator() 
    (AbstractObjectWithStates* object,
     const StateAxis& ax,
     const UniversalState& new_state)
  {}
};

template <
  class ClassAxis, 
  const char* initial,
  class StateHook = EmptyStateHook<ClassAxis, initial>
>
class ClassWithStates
{
public:
  virtual ~ClassWithStates() {}

  ClassWithStates* get_paired() //const
  { return paired; }

protected:
  /** States implementation. It is in an inner class
   * because we won't mix class states with object states
   * (class states are like state of repository or another
   * structure compound from separate members - classes.
   */
  class TheClass
    : public ObjectWithStatesInterface<ClassAxis>
  {
  public:
    void state_changed
      (StateAxis& ax, 
       const StateAxis& state_ax,     
       AbstractObjectWithStates* object,
       const UniversalState& new_state) override;

    std::atomic<uint32_t>& 
      current_state(const StateAxis& ax) override;

    const std::atomic<uint32_t>& 
      current_state(const StateAxis& ax) const override;
  };

  //! Paired obj ptr, for example, in the middle of a move
  //! constructor. It's used in descendants but declared
  //! here to access from StateHook-s (without dynamic
  //! casting).
  ClassWithStates* paired = nullptr;

public:
  //! The class state.
  virtual TheClass& the_class() const = 0;
};

template <
  class ClassAxis, 
  const char* initial, 
  class StateHook = EmptyStateHook<ClassAxis, initial>
>
class ClassWithEvents 
  : public ClassWithStates<ClassAxis, initial, StateHook>
{
public:
  typedef ClassWithStates<ClassAxis, initial, StateHook>
    Parent;

protected:
  class TheClass 
    : public Parent::TheClass,
      public ObjectWithEventsInterface<ClassAxis>
  {
  public:
    CompoundEvent create_event
     (const UniversalEvent&) const override;

    void update_events
      (StateAxis& ax, 
      TransitionId trans_id, 
      uint32_t to) override;

  protected:
    //! It maps a local event id to an Event object
    typedef std::map<uint32_t, Event> EventMap;

    //! Get static events collection. <NB> const - the
    //! collection is mutable.
    EventMap& get_events() const
    {
      static EventMap events;
      return events;
    }
  };
};

//! @}

}
#endif


