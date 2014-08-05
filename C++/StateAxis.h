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

#ifndef CONCURRO_STATEAXIS_H_
#define CONCURRO_STATEAXIS_H_

#include <iostream>
#include <atomic>
#include <mutex> // for axis singleton
#include <exception>

//#include "types/meta.h"
#include "types/string.h"

namespace curr {

//! @addtogroup exceptions
//! @{

struct StateAxisException : std::exception {};

//! @}

//! @addtogroup states
//! @{

typedef uint16_t TransitionId;

class AbstractObjectWithStates;
class AbstractObjectWithEvents;
class StateMap;
class UniversalState;

//! A state space axis abstract base. Real axises will be
//! inherited.
template<char... cs>
struct StateAxis 
{
  using name = ::types::meta_string<cs...>;

  StateAxis(::types::constexpr_string a_name) : name(a_name)
  {}

  virtual ~StateAxis() {}

  virtual const std::atomic<uint32_t>& current_state
  (const AbstractObjectWithStates*) const;

  virtual std::atomic<uint32_t>& current_state
  (AbstractObjectWithStates*) const;

  virtual void update_events
  (AbstractObjectWithEvents* obj, 
   TransitionId trans_id, 
   uint32_t to);

  virtual void state_changed
  (AbstractObjectWithStates* subscriber,
   AbstractObjectWithStates* publisher,
   const StateAxis& state_ax,
   const UniversalState& new_state);

  //! Change StateMap in us to the map of the axis.
  virtual UniversalState bound(uint32_t st) const = 0;

  virtual const StateAxis* vself() const = 0;
};

template<class Axis>
inline bool is_same_axis(const StateAxis& ax)
{
  return Axis::is_same(ax);
}

#define DECLARE_AXIS0(axis, parent_tn)	\
{ \
  typedef parent_tn Parent; \
  static axis self_; \
  static axis& self() { \
    static std::once_flag of; \
    static axis* self = nullptr; \
    std::call_once(of, [](){ self = new axis(#axis + parent_tn::name); });\
    assert(self); \
    return *self; \
  } \
  \
  static bool is_same(const StateAxis& ax) { \
    return &axis::self() == ax.vself();        \
  } \
  \
  static curr::StateMapPar<axis> get_state_map_par(); \
  \
  const curr::StateAxis* vself() const override \
  { \
    return &axis::self(); \
  } \
  \
  const std::atomic<uint32_t>& current_state \
    (const curr::AbstractObjectWithStates* obj) \
      const override; \
  \
  std::atomic<uint32_t>& current_state \
    (curr::AbstractObjectWithStates* obj) const override;\
  \
  void update_events \
    (curr::AbstractObjectWithEvents* obj,       \
     curr::TransitionId trans_id,               \
     uint32_t to) override;                     \
  \
  void state_changed \
    (curr::AbstractObjectWithStates* subscriber,   \
     curr::AbstractObjectWithStates* publisher,    \
     const curr::StateAxis& state_ax, \
     const curr::UniversalState& new_state) override;    \
  \
  curr::UniversalState bound(uint32_t st) const override;\
}; 

#define DECLARE_AXIS(axis, parent) \
  struct axis : parent \
  DECLARE_AXIS0(axis, parent)

#define DECLARE_AXIS_TEMPL(axis, templ_base, parent) \
  template<class T, class Enable = void> \
  struct axis; \
  \
  template<class T> \
  struct axis<T, CURR_ENABLE_BASE_TYPE(templ_base, T)> :\
      parent \
  DECLARE_AXIS0(axis, typename parent)

//! @}

}
#endif

