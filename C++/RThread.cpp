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

#include <assert.h>
#include "Logging.h"
#include "RThread.hpp"
#include "RThreadRepository.hpp"
#include "SShutdown.h"
#include "REvent.hpp"
#include "RState.hpp"

namespace curr {

// RThread states  ========================================

DEFINE_AXIS(
  ThreadAxis,
  {  "ready",         // after creation
      "starting",      
      "working",       
      "terminated",
      "cancelled"
      },
  {
    {"ready", "starting"},      // start ()

    {"starting", "working"},    
    // from a user-overrided run() method

    {"working", "terminated"},  
    // exit from a user-overrided run() 
    // <NB> no ready->terminated, i.e.,
    // terminated means the run() was executed (once and
    // only once)

    {"ready", "cancelled"},
    // to the possibility of destroying non-started
    // threads (ticket:71)
    {"cancelled", "cancelled"}
  }
  );

DEFINE_STATES(ThreadAxis);

DEFINE_STATE_CONST(RThreadBase, ThreadState, ready);
DEFINE_STATE_CONST(RThreadBase, ThreadState, starting);
DEFINE_STATE_CONST(RThreadBase, ThreadState, working);
DEFINE_STATE_CONST(RThreadBase, ThreadState, terminated);
DEFINE_STATE_CONST(RThreadBase, ThreadState, cancelled);

bool RThreadBase::LogParams::current = false;

RThreadBase::RThreadBase 
  (const std::string& id, 
   Event* extTerminated,
   const std::string& name
  )
: 
  RObjectWithEvents<ThreadAxis> (readyState),
  CONSTRUCT_EVENT(starting),
  CONSTRUCT_EVENT(terminated),
  CONSTRUCT_EVENT(cancelled),
  universal_object_id(id),
  thread_name((name.empty()) ? id : name),
  destructor_delegate_is_called(false),
  isStopRequested
  (SFORMAT("RThreadBase[id=" << id 
           << "]::isStopRequested"), 
   true, false),
  is_terminal_state_event {
    is_terminated_event, is_cancelled_event
  },
  externalTerminated (extTerminated)
{
  LOG_DEBUG (log, "thread " << pretty_id() 
             << "\t is created [" 
             << id << "]");
}

RThreadBase::RThreadBase
  (const ObjectCreationInfo& oi, const Par& p)
: 
    RThreadBase
      (oi.objectId, p.extTerminated, p.thread_name)
{
}

RThreadBase& RThreadBase::start ()
{
  ThreadState::move_to (*this, startingState);
  start_impl ();
  return *this;
}

void RThreadBase::stop()
{
  isStopRequested.set ();
}

bool RThreadBase::cancel()
{
  return compare_and_move
    (*this, {readyState, cancelledState}, cancelledState);
}

std::thread::native_handle_type RThread<std::thread>::Par
//
::get_id(ObjectCreationInfo& oi) const
{
  repository = dynamic_cast<RepositoryType*>
    (oi.repository);
  SCHECK(repository);
  oi.objectCreated = &rthreadCreated;

  if (th_id)
    return th_id;

  th = std::unique_ptr<std::thread>
    (new std::thread
     (&RThread<std::thread>::Par::run0, 
      const_cast<Par*>(this)));
  th_id = th->native_handle();
  LOG_DEBUG(log, "thread " << th_id 
            << " is created by " << par_num << "par");
  return th_id;
}


std::thread::id RThread<std::thread>::main_thread_id; 
// the default value not represents a thread

RThread<std::thread>* RThread<std::thread>
::current ()
{
#ifdef _WIN32
  return reinterpret_cast<RThread*> (_current.get ());
#else
#if 0
  return _current;
#else
  // it's ugly, but seams no another way
  const auto native_handle =
    fromString<std::thread::native_handle_type>
    (toString(std::this_thread::get_id()));

  try
  {
    return dynamic_cast<RThread<std::thread>*>
      (StdThreadRepository
       ::instance()
       . get_object_by_id (native_handle));
  }
  catch (const StdThreadRepository
         ::NoSuchId&)
  {
    LOG_DEBUG_STATIC_PLACE(log, current, 
                           "std::thread[native_handle="
                           << native_handle 
                           << "] is not registered "
                           " in the thread repository");
    return nullptr;
  }
#endif
#endif
}

std::string RThread<std::thread>
::current_pretty_id()
{
  if (std::this_thread::get_id() == main_thread_id)
    return std::string("main");

  auto* cur = current();
  if (cur) {
    return cur->pretty_id();
  }
  else
    return toString(std::this_thread::get_id());
}


#if 0
unsigned int __stdcall RThread::_helper( void * p )
{
  RThread * _this = reinterpret_cast<RThread *>(p);
  _current.set (_this);
  _this->_run();
  return 0;
}
#endif

void RThreadBase::outString (std::ostream& out) const
{
  out << "RThread(id = ["  
      << universal_object_id
      << "], this = " 
      << std::hex << (void *) this << std::dec
      << ", currentState = " 
      << RState<ThreadAxis>(*this) << ')';
}

//std::atomic<int> RThreadBase::counter (0);
 

// For proper destroying in concurrent environment
// 1) nobody may hold RThread* and descendants (even temporary!), // only access through Repository is allowed
// FIXME !!!

RThreadBase::~RThreadBase()
{
  LOG_DEBUG(log, "thread "
            << RThread<std::thread>::current_pretty_id()
            << ">\t ~RThreadBase");
  if (!destructor_delegate_is_called)
    THROW_PROGRAM_ERROR;
  // must be called from desc. destructors
}


void RThreadBase::destroy()
{
  if (destructor_delegate_is_called) 
    return;

  if (!cancel()) {
    isStopRequested.set();
    is_terminated_event.wait();
  }
    
  LOG_DEBUG (log, "thread " << pretty_id() 
             << ">\t destroyed");
  destructor_delegate_is_called = true;
}


void RThreadBase::_run()
{
  (is_starting_event | is_cancelled_event).wait();
  if (is_cancelled_event.signalled()) {
    LOG_DEBUG (log, "thread " << pretty_id() 
               << ">\t cancelled");
    return;
  }

  //TODO check run from current thread
  LOG_DEBUG (log, "thread " << pretty_id() 
             << ">\t started");
  try
  {
    run();
  }
  catch ( XShuttingDown & )
  {
    // Do nothing - execution is finished
  }
  catch ( std::exception & x )
  {
    // TODO make an additional state
    // "terminated_by_exception", store the exception.
    LOG_WARN(log, "Exception in thread " 
      << pretty_id() << " : "
      << x.what ());
  }
  catch ( ... )
  {
    LOG_WARN(log, 
      "Unknown type of exception in thread "
      << pretty_id()
    );
  }

  LOG_DEBUG (log, "thread " << pretty_id() << ">\t finished");

  if (externalTerminated) 
    externalTerminated->set ();

  ThreadState::move_to (*this, terminatedState);
  // no code after that (destructor can be called)
}

void RThread<std::thread>::remove(bool freeMemory)
{
  StdThreadRepository::instance()
    . delete_thread(this, freeMemory);

}

// LightThread

LightThread::LightThread
  (const ObjectCreationInfo& oi, const Par& par)
    : RThread<std::thread>(oi, par), fun(par.fun)
{
  assert(fun);
}

void LightThread::run()
{
  STATE(LightThread, move_to, working);
  fun();
}

}
