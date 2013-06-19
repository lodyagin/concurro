// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_EVENT_H
#define CONCURRO_EVENT_H

#include "SCheck.h"
#include "Logging.h"
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
#include <assert.h>

class EventInterface
{
public:
  virtual ~EventInterface() {}

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  virtual bool wait(int time = -1) = 0;
  virtual bool wait(int time = -1) const = 0;

  virtual bool signalled() const = 0;
};

class EvtBase : public EventInterface
{
  friend class Event;
  friend class CompoundEvent;
public:
  struct LogParams {
    bool set, reset;
    log4cxx::LoggerPtr logger;

    LogParams() : set(true), reset(true), 
      logger(Logger<LOG::Events>::logger()) {}
  } log_params;

  EvtBase(const EvtBase&) = delete;
  virtual ~EvtBase();

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  bool wait(int time = -1)
  { 
    return wait_impl(time); 
  }

  //! Wait for event or time in msecs. This is a const
  //! version  - for manual-reset events only.
  //! \return false on timeout.
  bool wait(int time = -1) const
  {
    SCHECK(is_manual);
    bool returnValue = wait_impl(time);
    isSignaled = is_manual ? isSignaled.load() : false;
    return returnValue;
  }

  void set();
  void reset();

  bool signalled() const
  {
    return isSignaled;
  }

  bool get_shadow() const
  {
    return shadow;
  }

  //! It is like wait(int) but return true if shadow is
  //! true (i.e., the event "was")
  bool wait_shadow(int time = -1)
  {
    //LOG_DEBUG(log, "shadow = " << shadow);
    return shadow || wait_impl(time);
  }

  bool wait_shadow(int time = -1) const
  {
    SCHECK(is_manual);
    //LOG_DEBUG(log, "shadow = " << shadow);
    return shadow || wait_impl(time);
  }

  const std::string universal_object_id;

protected:
  //! It is set if the event was occured at least once.
  std::atomic<bool> shadow;

  mutable std::atomic_bool isSignaled;

  const bool is_manual;
  HANDLE h;

  EvtBase(const std::string& id, bool manual, bool init);

  bool wait_impl(int time) const;
};

std::ostream&
operator<< (std::ostream&, const EvtBase&);

//! A windows-like event class.
class Event : public EventInterface
{
  friend class CompoundEvent;
public:

  //! Share an internal event handler
  Event(const Event& e) : evt_ptr(e.evt_ptr) {}
  Event(Event&& e) : evt_ptr(std::move(e.evt_ptr)) {}

  explicit Event
    (const std::string& id, 
     bool manual, //! manual reset
     bool init = false //! initial state 
      )
    : evt_ptr(new EvtBase(id, manual, init)) {}

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

  bool wait(int time = -1)
  { 
    return evt_ptr->wait(time); 
  }

  bool wait(int time = -1) const
  {
    return evt_ptr->wait(time); 
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

  EvtBase::LogParams& log_params() const
  {
    return evt_ptr->log_params;
  }

protected:
  std::shared_ptr<EvtBase> evt_ptr;

private:
  typedef Logger<LOG::Events> log;
};

DEFINE_EXCEPTION(
  AutoresetInCompound,
  "Unable to have an autoreset event "
  "as a member of CompoundEvent");

#define STL_BUG 1

class CompoundEvent : public EventInterface
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

  bool wait(int time = -1)
  {
    return wait_impl(time);
  }

  bool wait(int time = -1) const
  {
    return wait_impl(time);
  }

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

  bool wait_impl(int time) const;

  //! update the vector by the set if vector_need_update
  //! == true.
  void update_vector() const;

private:
  typedef Logger<LOG::Events> log;
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

#endif
