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

#ifndef CONCURRO_EXISTENT_H_
#define CONCURRO_EXISTENT_H_

#include "RObjectWithStates.h"
#include <atomic>

namespace curr {

DECLARE_AXIS(ExistenceAxis, StateAxis);

/**
 * Every object has two main states - exist and not_exist
 * which forms the ExistenceAxis. An object should be
 * derived from Existent if we need observe class copy
 * existence (e.g., in SSingleton).
 */
template<class T>
class Existent
{
  friend class Existent::Watcher;
public:
  /**
   * This is a "metaclass" for watch an Existent existence.
   *
   * @dot
   * digraph {
   *   start [shape = point]; 
   *   not_exist [shape = doublecircle];
   *   start -> not_exist;
   *   not_exist -> exist_one
   *     [label="notify_construction()"];
   *   exist_one -> exist_several
   *     [label="notify_construction()"];
   *   exist_several -> exist_several
   *     [label="notify_{construction,destruction}()"];
   *   exist_several -> exist_one
   *     [label="notify_destruction()"];
   *   exist_one -> not_exist
   *     [label="notify_destruction()"];
   * }
   * @enddot
   *
   */
  class Watcher : public RObjectWithStates<ExistenceAxis>
  {
  public:
    //! @cond
    DECLARE_STATES(ExistenceAxis, State);
    DECLARE_STATE_CONST(State, not_exist);
    DECLARE_STATE_CONST(State, exist_one);
    DECLARE_STATE_CONST(State, exist_several);
    //! @endcond

    Watcher();
    virtual ~Watcher();

    //! Return a number of Existent objects
    unsigned get_obj_count() const
    { return obj_count; }

  protected:
    std::atomic<int> obj_count;

    virtual void notify_construction();
    virtual void notify_destruction();
  };

  static Watcher watcher;

  Existent();
  Existent(const Existent&);
  Existent(Existent&&);
  virtual ~Existent();
  Existent& operator=(const Existent&);
  Existent& operator=(Existent&&);
};

}

#endif
