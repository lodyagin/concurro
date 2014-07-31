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

#include <unistd.h>
#include <signal.h>
#include "Process.h"
#include "SSingleton.hpp"
#include "SCommon.h"

namespace curr {

template class SAutoSingleton<ChildProcessRepository>;

DEFINE_AXIS(
  ProcessAxis,
  {  "stopped",
     "working",
     "terminated"
  },
  {
    {"stopped", "working"},
    {"working", "stopped"},
    {"working", "terminated"}
  }
);

DEFINE_STATE_CONST(Process, State, stopped);
DEFINE_STATE_CONST(Process, State, working);
DEFINE_STATE_CONST(Process, State, terminated);

pid_t Process::Par::get_id(ObjectCreationInfo& oi) const
{
  const pid_t fork_pid = ::fork();

  if (fork_pid == 0) { // the child
    const pid_t pid = ::getpid();
    const int kill_res = ::kill(pid, /*19 */ SIGSTOP);
    assert(kill_res == 0);
    assert(false);
  }
  else return fork_pid;
}

Process::Process(
  const ObjectCreationInfo& oi, 
  const Par& par
)
  : RObjectWithEvents(stoppedState),
    CONSTRUCT_EVENT(terminated),
    id(fromString<pid_t>(oi.objectId)),
    is_terminal_state_event{ is_terminated_event }
{
}

void Process::destroy()
{
  if (destructor_delegate_is_called) 
    return;

  is_terminal_state_event.wait();
    
  LOG_DEBUG(log, "process " << id << "exits");
  destructor_delegate_is_called = true;
}

std::ostream&
operator<<(std::ostream& out, const Process& proc)
{
  return out << "Process{pid: " << proc.id << '}';
}

} // curr
