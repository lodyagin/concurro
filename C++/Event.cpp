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

#include "types/exception.h"
#include "Logging.h"
#include "Event.h"
#include "SCheck.h"
#include "RThread.h"
#ifdef _WIN32
#include "SShutdown.h"
#else
using namespace neosmart;
#endif
#include "SSingleton.hpp"

namespace curr {

[[noreturn]] void EventWaitingTimedOut::raise()
{
  throw ::types::exception(
    *this,
    "Event waiting timed out after ", msecs," milliseconds"
  );
}

[[noreturn]] void AutoresetInCompound::raise()
{
  throw ::types::exception<AutoresetInCompound>(
    "Unable to have an autoreset event "
    "as a member of a CompoundEvent"
  );
}

// EvtBase  ===============================================

EvtBase::EvtBase(
  const std::string& id, 
  bool manual, 
  bool init,
  bool logging
)
  : universal_object_id(id),
    shadow(false),
    isSignaled(init),
    is_manual(manual),
#ifdef _WIN32
    h(CreateEvent(0, manual, init, 0))
#else
    h(CreateEvent(manual, init))
#endif
{
  if (!logging)
    log_params().disable
      <place::set, place::reset, place::wait>();

#if 0 // FIXME recursion (add create to log_params?)
  LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t event "
            << universal_object_id 
            << ">\t created in state " << init);
#endif
#ifdef _WIN32
  sWinCheck(h != 0, L"creating an event");
#endif
}

EvtBase::~EvtBase()
{
#if 0 // crash in Logger destruction
  LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t closes the event handle [" 
            << universal_object_id << "]");
#endif

#ifdef _WIN32
  if (!h) CloseHandle(h);
#else
  if (h) DestroyEvent(h);
#endif
  h = 0; 
}

logging::LoggerPtr EvtBase::logger() const 
{
  return Logger<LOG::Events>::s_logger();
}

std::ostream&
operator<< (std::ostream& out, const EvtBase& evt)
{
  out << '[' << evt.universal_object_id << ']';
  return out;
}

void EvtBase::set()
{
  LOGGER_DEBUG_PLACE(log_params().log_obj->logger(), 
   place::set, "thread " 
                << RThread<std::thread>::current_pretty_id()
                  << ">\t event "
                  << universal_object_id << ">\t set");
#ifdef _WIN32
  sWinCheck
    (SetEvent(h) != 0, 
     SFORMAT (L"setting event, handle = " << h).c_str ()
      );
#else
  isSignaled = true;
  shadow = true; //<NB> before SetEvent to prevent a "no
  //shadow and no event" case
  SetEvent(h);
#endif
}

void EvtBase::reset()
{
  LOGGER_DEBUG_PLACE(log_params().log_obj->logger(), 
    place::reset, "thread " 
                << RThread<std::thread>::current_pretty_id()
                  << ">\t event "
                  << universal_object_id << ">\t reset");
#ifdef _WIN32
  sWinCheck
    (ResetEvent(h) != 0, 
     SFORMAT (L"resetting event, handle = " << h).c_str ()
      );
#else
  //is_signalled = false;
  ResetEvent(h);
#endif
  isSignaled = false;
}

bool EvtBase::wait_impl(int time) const
{
  if (!log_params()[place::wait()]) {
    if (time != -1) {
      LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
                   << RThread<std::thread>::current_pretty_id()
                   << ">\t event "
                   << universal_object_id 
                   << ">\t wait " << time << " msecs");
    }
    else {
      LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
                   << RThread<std::thread>::current_pretty_id()
                   << ">\t event "
                   << universal_object_id 
                   << ">\t waits w/o timeout");
    }
  }

  HANDLE evts[] = {
#ifndef SHUTDOWN_UNIMPL
    SShutdown::instance().event(),
#endif
    h };
#ifdef _WIN32
  DWORD code = WaitForMultipleObjects(2, evts, false, time);
  if ( code == WAIT_OBJECT_0 ) xShuttingDown(L"REvent.wait");
  if ( code == WAIT_TIMEOUT ) return false;
  if ( code != WAIT_OBJECT_0 + 1 ) 
    sWinErrorCode(code, L"waiting for an event");
#else
  int code = WaitForEvent(evts[0], time);
  if (code == ETIMEDOUT) {
    if (!log_params()[place::wait()]) {
      LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
              << RThread<std::thread>::current_pretty_id()
              << ">\t event "
              << universal_object_id 
              << ">\t wait: timed out");
    }
    return false;
  }
#endif
  if (!log_params()[place::wait()]) {
    LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t event "
            << universal_object_id 
            << ">\t wait: signalled");
  }
  return true;
}

void EvtBase::wait(
  int time, 
  const EventWaitingTimedOut&
)
{ 
  if (!wait(time))
    EventWaitingTimedOut(time).raise();
}

void EvtBase::wait(
  int time, 
  const EventWaitingTimedOut&
) const 
{ 
  if (!wait(time))
    EventWaitingTimedOut(time).raise();
}

bool EvtBase::wait(int time) const
{
  SCHECK(is_manual);
  bool returnValue = wait_impl(time);
  isSignaled = is_manual ? isSignaled.load() : false;
  return returnValue;
}

bool EvtBase::wait_shadow(int time)
{
  return shadow || wait_impl(time);
}

bool EvtBase::wait_shadow(int time) const
{
  SCHECK(is_manual);
  return shadow || wait_impl(time);
}

CompoundEvent::CompoundEvent()
  : vector_need_update(false), //<NB>
    update_vector_mx("CompoundEvent::update_vector_mx")
{}

#if 0
CompoundEvent::CompoundEvent(CompoundEvent&& e)
  : handle_set(std::move(e.handle_set)),
    handle_vec(std::move(e.handle_vec)),
    vector_need_update(e.vector_need_update),
    update_vector_mx("CompoundEvent::update_vector_mx")
{
//  using log = Logger<LOG::Events>;
  //commented out to prevent an infinite loop
  //LOG_TRACE(log, "CompoundEvent::move constructor");
}
#endif

CompoundEvent::CompoundEvent(const CompoundEvent& e)
#if !STL_BUG
  : handle_set(e.handle_set),
    handle_vec(e.handle_vec),
    vector_need_update(e.vector_need_update),
    update_vector_mx("CompoundEvent::update_vector_mx")
{
  using log = Logger<LOG::Events>;
  LOG_TRACE(log, "CompoundEvent::copy constructor");
}
#else
  : handle_vec(e.handle_vec),
    vector_need_update(e.vector_need_update),
    update_vector_mx("CompoundEvent::update_vector_mx")
{
  //using log = Logger<LOG::Events>;
  handle_set = e.handle_set;
  //commented out to prevent an infinite loop
//  LOG_TRACE(log, "CompoundEvent::copy constructor");
}
#endif

CompoundEvent::CompoundEvent(const Event& e)
  : handle_set{e}, 
    vector_need_update(true),
    update_vector_mx("CompoundEvent::update_vector_mx")
{
  if (!e.is_manual())
    AutoresetInCompound().raise();
}

CompoundEvent::CompoundEvent(
  std::initializer_list<Event> evs
)
  : vector_need_update(evs.size() > 0),
    update_vector_mx("CompoundEvent::update_vector_mx")
{
  for (const Event& e : evs) {
    handle_set.insert(e);
    if (!e.is_manual())
      AutoresetInCompound().raise();
  }
}

CompoundEvent::CompoundEvent
(std::initializer_list<CompoundEvent> evs)
  : vector_need_update(evs.size() > 0),
    update_vector_mx("CompoundEvent::update_vector_mx")
{
  for (const CompoundEvent& e : evs) {
    handle_set.insert(e.begin(), e.end());
  }
}

#if 0
CompoundEvent& CompoundEvent
::operator= (CompoundEvent&& e)
{
  using log = Logger<LOG::Events>;
  LOG_TRACE(log, "CompoundEvent::operator=(move)");
  handle_set = std::move(e.handle_set);
  handle_vec = std::move(e.handle_vec);
  vector_need_update = e.vector_need_update;
  return *this;
}
#endif

//#if !STL_BUG
CompoundEvent& CompoundEvent
::operator= (const CompoundEvent& e)
{
  using log = Logger<LOG::Events>;
  LOG_TRACE(log, "CompoundEvent::operator=(copy)");
  handle_set = e.handle_set;
  vector_need_update = true; // <NB>
  return *this;
}
//#endif

const CompoundEvent& CompoundEvent
::operator|= (const Event& e)
{
  handle_set.insert(e);
  vector_need_update = true;
  if (!e.is_manual())
    AutoresetInCompound().raise();
  return *this;
}

CompoundEvent& CompoundEvent
::operator|= (const CompoundEvent& e)
{
  handle_set.insert(e.handle_set.begin(), 
                    e.handle_set.end());
  vector_need_update = true;
  return *this;
}

bool CompoundEvent
::operator< (const CompoundEvent& b) const
{
  const int diff = handle_set.size() - b.handle_set.size();
  if (diff < 0)
    return true;
  else if (diff > 0)
    return false;
  
  auto mis = std::mismatch(handle_set.begin(), 
                           handle_set.end(),
                           b.handle_set.begin());
  if (mis.first == handle_set.end())
    return false; // it equals
  return *mis.first < *mis.second;
}

bool CompoundEvent::signalled() const
{
  if (handle_set.empty())
    return true;
  for(auto &i : handle_set) {
    if (i.signalled()) 
      return true;
  }
  return false;
}

void CompoundEvent::wait(
  int time, 
  const EventWaitingTimedOut&
)
{
  if (!wait(time))
    EventWaitingTimedOut(time).raise();
}

void CompoundEvent::wait(
  int time, 
  const EventWaitingTimedOut&
) const
{
  if (!wait(time))
    EventWaitingTimedOut(time).raise();
}

bool CompoundEvent::wait_impl(int time) const
{
  if (!log_params()[place::wait()]) {
    if (time != -1) {
      LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
                   << RThread<std::thread>::current_pretty_id()
                   << ">\t event " << *this
                   << ">\t wait " << time << " msecs");
    }
    else {
      LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
                   << RThread<std::thread>::current_pretty_id()
                   << ">\t event " << *this
                   << ">\t waits w/o timeout");
    }
  }

  // an empty event is always signalled
  if (handle_set.empty())
    return true;

  update_vector();
  assert(handle_vec.size() > 0);

  const int code = WaitForMultipleEvents(&handle_vec[0], 
                                         handle_vec.size(),
                                         false, // wait for any
                                         time);

  if (code == ETIMEDOUT) {
    if (!log_params()[place::wait()]) {
      LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
              << RThread<std::thread>::current_pretty_id()
              << ">\t event " << *this
              << ">\t wait: timed out");
    }
    return false;
  }
  if (!log_params()[place::wait()]) {
    LOGGER_DEBUG(log_params().log_obj->logger(), "thread " 
            << RThread<std::thread>::current_pretty_id()
            << ">\t event " << *this
            << ">\t wait: signalled");
  }
  return true;
}

void CompoundEvent::update_vector() const
{
  if (!vector_need_update)
    return;
  { 
    RLOCK(update_vector_mx);

    handle_vec.clear();
    handle_vec.reserve(handle_set.size());
    std::transform
      (handle_set.begin(), handle_set.end(),
       std::back_inserter(handle_vec),
       [](const Event& e) { return e.evt_ptr->h; });

    // Merge disabled log params from all events
    LogParams& lp = ObjectWithLogParams::log_params();
    lp.enable_all();
    for(const Event& e : handle_set)
      lp |= e.log_params();
  }
  vector_need_update = false;
  assert(handle_vec.size() == handle_set.size());
}

std::ostream&
operator<< (std::ostream& out, const CompoundEvent& ce)
{
  out << "{";
  bool first = true;
  for (auto& ev : ce.handle_set) {
    if (!first) out << " | "; else first = false;
    out << ev.universal_id();
  }
  out << "}";
  return out;
}


// SSemaphore  =======================================================
/*
  SSemaphore::SSemaphore( int maxCount, int initCount ) :
  Parent(CreateSemaphore(0, initCount, maxCount, 0))
  {
  }

  void SSemaphore::release( int count )
  {
  sWinCheck(ReleaseSemaphore(h, count, 0) != 0, L"releasing semaphore");
  }
*/

//====================================================================

#if 0
size_t waitMultiple( HANDLE * evts, size_t count )
{
#ifdef _WIN32
  SPRECONDITION(count <= MAXIMUM_WAIT_OBJECTS);
  
  DWORD code = WaitForMultipleObjects(count, evts, false, INFINITE);
  
  if ( code < WAIT_OBJECT_0 || code >= WAIT_OBJECT_0 + count )
    sWinErrorCode(code, L"waiting for multiple objects");
  
  return code - WAIT_OBJECT_0;
#else
  int code = WaitForMultipleEvents(evts, count, false, 0);
  return code;// - WAIT_OBJECT_0;
#endif
}

size_t waitMultipleSD( HANDLE * _evts, size_t count )
{
#ifdef _WIN32
  SPRECONDITION(count <= MAXIMUM_WAIT_OBJECTS - 1);
  
  HANDLE evts[MAXIMUM_WAIT_OBJECTS];
#endif
#ifndef SHUTDOWN_UNIMPL
  evts[0] = SShutdown::instance().event();
  memcpy(evts + 1, _evts, count * sizeof(HANDLE));
  size_t idx = waitMultiple(evts, count + 1);
#else
  size_t idx = waitMultiple(_evts, count );
#endif

  if ( idx == 0 ) xShuttingDown("waitMultipleSD");
  return idx - 1;
}

#endif
}
