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
 * Process management.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_PROCESS_H_
#define CONCURRO_PROCESS_H_

#include <iostream>
#include <map>
#include <vector>
#include <sys/types.h>
#include "SSingleton.h"
#include "Repository.h"
#include "RObjectWithStates.h"
#include "Logging.h"

namespace curr {

DECLARE_AXIS(ProcessAxis, StateAxis);

//! A child process descriptor
class Process : public RObjectWithEvents<ProcessAxis>
{
  DECLARE_EVENT(ProcessAxis, terminated);

public:
  //! @cond
  DECLARE_STATES(ProcessAxis, State);
  DECLARE_STATE_CONST(State, stopped);
  DECLARE_STATE_CONST(State, working);
  DECLARE_STATE_CONST(State, terminated);
  //! @endcond 
 
  struct Par 
  {
    //! A command with args
    std::string cmd;

    //! Creates the process in stopped state
    pid_t get_id(ObjectCreationInfo& oi) const;

    Process* create_derivation(
      const ObjectCreationInfo& oi
    ) const
    {
      return new Process(oi, *this);
    }

    Process* transform_object(const Process*) const
    {
      throw ::types::exception
        <TransformObjectIsNotImplemented>();
    }
  };

  //! System PID
  const pid_t id;

  enum {
    stdin = 0,
    stdout = 1,
    stderr = 2
  };

  //! At least stdin, stdout, stderr must be present
  std::vector<std::iostream> streams;

  Process(const Process&) = delete;
  Process& operator=(const Process&) = delete;

  ~Process()
  {
    destroy();
  }

  CompoundEvent is_terminal_state() const override
  {
    return is_terminal_state_event;
  }

  std::string universal_id() const
  {
    return toString(id);
  }

  //! ready->working
  void start();

protected:
  Process(const ObjectCreationInfo& oi, const Par& par);

  //! A real work of a destructor is here. All descendants
  //! must call it in the destructor. It waits the
  //! terminationEvent. (Must be called from the most
  //! overrided destructor to prevent destroying of
  //! critical objects prior to the system process exits).
  void destroy();

private:
  using log = Logger<Process>;
  CompoundEvent is_terminal_state_event;
  bool destructor_delegate_is_called = false;
};

std::ostream&
operator<<(std::ostream& out, const Process& proc);

//! Repository of child processes
class ChildProcessRepository final
 : public SAutoSingleton<ChildProcessRepository>,
   public Repository<
     Process, Process::Par, std::map, pid_t
   >
{
  using Rep = Repository <
    Process, Process::Par, std::map, pid_t
  >;

public:
  ChildProcessRepository()
    : Rep("ChildProcessRepository", 0)
  {
    complete_construction();
  }
};

} // curr

#endif

