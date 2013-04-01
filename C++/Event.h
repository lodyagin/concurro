// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_EVENT_H
#define CONCURRO_EVENT_H

#ifndef _WIN32
#define WFMO
#include "pevents.h"
typedef neosmart::neosmart_event_t HANDLE;
#endif

class EvtBase
{
public:
  //! Wait for event.
  virtual void wait();
  //! Wait for event or time in msecs. 
  //! \return false on a timeout.
  virtual bool wait( int time );  
  virtual ~EvtBase();

  // Direct access not allowed due to combined event logic
  // possibility (i.e. atomic set the event and additional
  // info). 
  //HANDLE evt()  { return h; }

protected:

  EvtBase( HANDLE );

  HANDLE h;

};

//! A windows-like event class.
class Event : public EvtBase
{
public:

  typedef EvtBase Parent;

  explicit Event
	 (bool manual, //! manual reset
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
