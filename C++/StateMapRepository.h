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

#ifndef CONCURRO_STATEMAPREPOSITORY_H_
#define CONCURRO_STATEMAPREPOSITORY_H_

#include "StateMap.h"
#include "SSingleton.h"

namespace curr {

//! @addtogroup states
//! @{

/**
 * A repository for state maps. It also maintains
 * additional global data.
 */
class StateMapRepository 
: public Repository<
  StateMap, 
  StateMapParBase, 
  std::unordered_map,
  StateMapId
  >
{
  friend class StateMap;
  friend class StateMapParBase;
public:
  typedef Repository< 
    StateMap, 
    StateMapParBase, 
    std::unordered_map,
    StateMapId > Parent;

  //! For id == 0 return empty map.
  StateMap* get_object_by_id
    (const StateMapId& id) const override;

  //! Return max used transition id
  TransitionId max_transition_id() const
  { return max_trans_id; }

  //! Return a state map by a state axis
  StateMap* get_map_for_axis(const std::type_info& axis);

  //! Return a map id by an axis type.
  //! It creates new StateMapId 
  //! if it is new (unregistered) axis.
  StateMapId get_map_id(const std::type_info& axis);

  std::string get_state_name(uint32_t state) const;

  static StateMapRepository& instance()
  {
    typedef StateMapRepository T;
    static std::once_flag of;
    static T* instance = nullptr;
    std::call_once(of, [](){ instance = new T(); });
    assert(instance);
    return *instance;
  }

protected:
  //! It is incremented in a StateMap constructor.
  std::atomic<TransitionId> max_trans_id;

  StateMapId last_map_id;
  std::map<std::string, StateMapId> axis2map_id;

  static StateMap* empty_map;

private:
  StateMapRepository() 
    : Parent("StateMapRepository", 50), 
      max_trans_id(0), last_map_id(0) 
  {}

  StateMapRepository(const StateMapRepository&) = delete;
  ~StateMapRepository() {};
  StateMapRepository& operator=
    (const StateMapRepository&) = delete;
};

//! @}

}
#endif


