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
#if __GNUC_MINOR__< 6
#include <cstdatomic>
#else
#include <atomic>
#endif
#include <ostream>
#include <vector>
#include <set>
#include <memory>

class EventInterface
{
public:
  virtual ~EventInterface() {}

  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  virtual bool wait(int time = -1) = 0;
  virtual bool wait(int time = -1) const = 0;

  //<NB> no signalled() - because in CompoundEvent auto
  //and manual reset events can be mixed (no signalled()
  //logic for auto-reset events).
};

class EvtBase : public EventInterface
{
  friend class Event;
  friend class CompoundEvent;
public:
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
	 return shadow || wait_impl(time);
  }

  bool wait_shadow(int time = -1) const
  {
	 SCHECK(is_manual);
	 return shadow || wait_impl(time);
  }

  const std::string universal_object_id;
protected:
  typedef Logger<LOG::Events> log;

  //std::atomic<bool> is_signalled;

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

protected:
  typedef Logger<LOG::Events> log;

  std::shared_ptr<EvtBase> evt_ptr;
};

class CompoundEvent : public EventInterface
{
  friend std::ostream&
  operator<< (std::ostream&, const CompoundEvent&);

public:
  CompoundEvent();
  CompoundEvent(CompoundEvent&&); //UT+
  CompoundEvent(const CompoundEvent&); //UT+
  CompoundEvent(const Event&); //UT+
  CompoundEvent(std::initializer_list<Event>);

  CompoundEvent& operator= (CompoundEvent&&);

  const CompoundEvent& operator|= (const Event&); //UT+
  //CompoundEvent& operator|= (const CompoundEvent&);

  bool wait(int time = -1)
  {
	 return wait_impl(time);
  }

  bool wait(int time = -1) const
  {
	 SCHECK(!has_autoreset);
	 return wait_impl(time);
  }

  bool isSignaled(){
  	for(auto &i : handle_set)
  		if (i.signalled()) return true;
  	return false;
  }

  //! A number of unique events inside.
  size_t size() const
  {
	 return handle_set.size();
  }

protected:
  typedef Logger<LOG::Events> log;

  //! a set for accumulate handles
  std::set<Event> handle_set;
  //! a vector to pass to WaitForMultipleEvents
  mutable std::vector<HANDLE> handle_vec;
  //! the vector need to be updated by the set
  mutable bool vector_need_update;
  //! at least one event has autoreset
  bool has_autoreset;

  bool wait_impl(int time) const;

  //! update the vector by the set if vector_need_update
  //! == true.
  void update_vector() const;
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

#if 0
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

#if 0
inline CompoundEvent operator| 
  (CompoundEvent a, CompoundEvent&& b)
{
  a |= b; return a;
}
#endif

#endif
