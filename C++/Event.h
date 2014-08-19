/* -*-coding: mule-utf-8-unix; fill-column: 58; -*- *******

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

#ifndef CONCURRO_EVENT_H
#define CONCURRO_EVENT_H

#include <utility>
#include "ObjectWithLogging.h"
#ifndef _WIN32
#define WFMO
#include "pevents.h"
typedef neosmart::neosmart_event_t HANDLE;
#endif
#include <atomic>
#include <ostream>
#include <vector>
#include <set>
#include <memory>
#include <exception>
#include <assert.h>

#define SHUTDOWN_UNIMPL

namespace curr {

/**
 * @addtogroup exceptions
 * @{
 */

struct EventException : virtual std::exception {};

struct EventWaitingTimedOut : EventException 
{
  const int msecs;
  EventWaitingTimedOut(int ms) : msecs(ms) {}
  [[noreturn]] void raise();
};

struct CompoundEventException : EventException {};

/**
 * Exception: unable to have an autoreset event 
 * as a member of CompoundEvent.
 */
struct AutoresetInCompound : CompoundEventException
{
  [[noreturn]] void raise();
};

/**
 * @defgroup events
 *
 * All about events.
 * @{
 */

#define CURR_WAIT_L(logger, evt, time)          \
  do { (evt).wait((time),                       \
       curr::EventWaitingTimedOut(time) \
       ); } while(false)

//! Call a timed event waiting which can throw
//! EventWaitingTimedOut with the location equal to the
//! macro substitution line.
#define CURR_WAIT(evt, time) \
  CURR_WAIT_L(log::s_logger(), evt, time)

class EventInterface //: public virtual ObjectWithLogging
{
public:
  struct place
  {
    struct set{}; struct reset{}; struct wait{};
  };

  virtual ~EventInterface() {}

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  virtual bool wait(int time = -1) = 0;
  
  //! Wait for event or time in msecs. 
  //! @throw EventWaitingTimedOut
  virtual void wait(
    int time, 
    const EventWaitingTimedOut&
  ) = 0;

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  virtual bool wait(int time = -1) const = 0;

  //! Wait for event or time in msecs. 
  //! @throw EventWaitingTimedOut
  virtual void wait(
    int time, 
    const EventWaitingTimedOut&
  ) const = 0;

  virtual bool signalled() const = 0;
};

class EvtBase 
  : public virtual EventInterface,
    public virtual ObjectWithLogging,
    public virtual ObjectWithLogParams
{
  friend class Event;
  friend class CompoundEvent;

public:
  /*struct place { 
    struct set{}; struct reset{}; struct wait{}; 
  };*/

  EvtBase(const EvtBase&) = delete;
  EvtBase& operator=(const EvtBase&) = delete;

  virtual ~EvtBase();

  logging::LoggerPtr logger() const override;

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  bool wait(int time = -1) override
  { 
    return wait_impl(time); 
  }

  void wait(int time, const EventWaitingTimedOut&) override;

  //! Wait for event or time in msecs. This is a const
  //! version  - for manual-reset events only.
  //! \return false on timeout.
  bool wait(int time = -1) const;

  void wait(int time, const EventWaitingTimedOut&) const 
    override;

  void set();
  void reset();

  bool signalled() const override
  {
    return isSignaled;
  }

  bool get_shadow() const
  {
    return shadow;
  }

  //! It is like wait(int) but return true if shadow is
  //! true (i.e., the event "was")
  bool wait_shadow(int time = -1);

  bool wait_shadow(int time = -1) const;

  const std::string universal_object_id;

protected:
  //! It is set if the event was occured at least once.
  std::atomic<bool> shadow;

  mutable std::atomic_bool isSignaled;

  const bool is_manual;
  HANDLE h;

  EvtBase(
    const std::string& id, 
    bool manual, 
    bool init,
    bool logging
  );

  bool wait_impl(int time) const;
};

std::ostream&
operator<< (std::ostream&, const EvtBase&);

//! A windows-like event class.
class Event : public virtual EventInterface
{
  friend class CompoundEvent;
public:

  //! Share an internal event handler
  Event(const Event& e) : evt_ptr(e.evt_ptr) {}
  Event(Event&& e) : evt_ptr(std::move(e.evt_ptr)) {}

  explicit Event(
    const std::string& id, 
    bool manual, //< manual reset
    bool init = false, //< initial state 
    bool logging = true
    )
    : evt_ptr(
        new EvtBase(id, manual, init, logging)
      ) 
  {}

  //! Share an internal event handler
  Event& operator= (const Event& e)
    {
      evt_ptr = e.evt_ptr;
      return *this;
    }

  Event& operator= (Event&& e)
    {
      evt_ptr = std::move(e.evt_ptr);
      return *this;
    }

  bool operator< (const Event& b) const
  {
    return evt_ptr->h < b.evt_ptr->h;
  }

  bool operator== (const Event& b) const
  {
    return evt_ptr->h == b.evt_ptr->h;
  }

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  bool wait(int time = -1) override
  { 
    return evt_ptr->wait(time); 
  }

  void wait(int time, const EventWaitingTimedOut& te) 
    override
  { 
    evt_ptr->wait(time, te); 
  }

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  bool wait(int time = -1) const override
  {
    return evt_ptr->wait(time); 
  }

  void wait(int time, const EventWaitingTimedOut& te) 
    const override
  {
    evt_ptr->wait(time, te); 
  }

  bool wait_shadow(int time = -1)
  {
    return evt_ptr->wait_shadow(time); 
  }

  bool wait_shadow(int time = -1) const
  {
    return evt_ptr->wait_shadow(time); 
  }

  bool get_shadow() const
  {
    return evt_ptr->get_shadow();
  }

  void set()
  {
    return evt_ptr->set();
  }

  void reset()
  {
    return evt_ptr->reset();
  }

  bool signalled() const
  {
    return evt_ptr->signalled();
  }

  bool is_manual() const
  {
    return evt_ptr->is_manual;
  }

  std::string universal_id() const
  {
    return evt_ptr->universal_object_id;
  }

  LogParams& log_params() const //override
  {
    return evt_ptr->log_params();
  }

protected:
  std::shared_ptr<EvtBase> evt_ptr;

private:
//  typedef Logger<LOG::Events> log;
};

#define STL_BUG 1

class CompoundEvent 
  : public virtual EventInterface,
    public virtual ObjectWithLogParams
{
  friend std::ostream&
    operator<< (std::ostream&, const CompoundEvent&);

public:
  CompoundEvent();
  CompoundEvent(CompoundEvent&&); //UT+
#if 1 //!STL_BUG
  CompoundEvent(const CompoundEvent&); 
#else
  CompoundEvent(const CompoundEvent&) = delete;
#endif
  CompoundEvent(const Event&); //UT+
  CompoundEvent(std::initializer_list<Event>);
  CompoundEvent(std::initializer_list<CompoundEvent>);

  CompoundEvent& operator= (CompoundEvent&&);
#if 1 //!STL_BUG
  CompoundEvent& operator= (const CompoundEvent&);
#else
  CompoundEvent& operator= (const CompoundEvent&) = delete;
#endif

  bool operator== (const CompoundEvent& b) const
  {
    return handle_set == b.handle_set;
  }

  bool operator< (const CompoundEvent& b) const;

  const CompoundEvent& operator|= (const Event&); //UT+
  CompoundEvent& operator|= (const CompoundEvent&);

  bool wait(int time = -1) override
  {
    return wait_impl(time);
  }

  bool wait(int time = -1) const
  {
    return wait_impl(time);
  }

  void wait(int time, const EventWaitingTimedOut&) override;

  void wait(int time, const EventWaitingTimedOut&) const
    override;

  bool signalled() const override;

  //! A number of unique events inside.
  size_t size() const
  {
    assert(vector_need_update
           || handle_vec.size() == handle_set.size());
    return handle_set.size();
  }

  std::set<Event>::iterator begin()
  {
    return handle_set.begin();
  }

  std::set<Event>::const_iterator begin() const
  {
    return handle_set.begin();
  }

  std::set<Event>::iterator end()
  {
    return handle_set.end();
  }

  std::set<Event>::const_iterator end() const
  {
    return handle_set.end();
  }

protected:
  //! a set for accumulate handles
  std::set<Event> handle_set;
  //! a vector to pass to WaitForMultipleEvents
  mutable std::vector<HANDLE> handle_vec;
  //! the vector need to be updated by the set
  mutable bool vector_need_update;

  //! wait time msecs
  bool wait_impl(int time) const;

  //! update the vector by the set if vector_need_update
  //! == true.
  void update_vector() const;

private:
//  typedef Logger<LOG::Events> log;
};

std::ostream&
operator<< (std::ostream&, const CompoundEvent&);

//hint: use operator& for wait for all
//! Append events for wait-for-any
inline CompoundEvent operator| 
  (const Event& a, const Event& b) //UT+
{
  CompoundEvent ca(a);
  ca |= b; return ca;
}

inline CompoundEvent operator| 
  (CompoundEvent a, const Event& b) //UT+
{
  a |= b; return a;
}

inline CompoundEvent operator| 
  (CompoundEvent a, const CompoundEvent& b)
{
  a |= b; return a;
}

inline CompoundEvent operator| 
  (const Event& a, CompoundEvent b)
{
  b |= a; return b;
}

#if 0
inline CompoundEvent operator| 
  (const CompoundEvent& a, CompoundEvent b)
{
  b |= a; return b;
}
#endif

inline CompoundEvent operator| 
  (Event&& a, Event&& b)
{
  CompoundEvent ca(a);
  ca |= b; return ca;
}

inline CompoundEvent operator| 
  (CompoundEvent a, CompoundEvent&& b)
{
  a |= b; return a;
}

//! @}

}
#endif
