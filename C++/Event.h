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
#ifndef GPP44
#include <atomic>
#else
#include <cstdatomic>
#endif
#include <ostream>

class EvtBase
{
public:
  //! Wait for event.
  virtual void wait();
  //! Wait for event or time in msecs. 
  //! \return false on timeout.
  virtual bool wait( int time );  
  virtual ~EvtBase();

  const std::string universal_object_id;
protected:
  typedef Logger<EvtBase> log;

  EvtBase(const std::string& id, HANDLE);

  HANDLE h;
};

std::ostream&
operator<< (std::ostream&, const EvtBase&);

//! A windows-like event class.
class Event : public EvtBase
{
public:

  typedef EvtBase Parent;

  explicit Event
	 (const std::string& id, 
     bool manual, //! manual reset
	  bool init = false //! initial state
		);
  ~Event(){}
  virtual void set();
  virtual void reset();

  bool signalled() const
  {
	 SCHECK(is_manual);
	 return is_signalled;
  }

protected:
  std::atomic<bool> is_signalled;
  const bool is_manual;
};

#endif
