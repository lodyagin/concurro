// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_REVENT_H
#define CONCURRO_REVENT_H

#ifndef _WIN32
#define WFMO
#include "pevents.h"
typedef neosmart::neosmart_event_t HANDLE;
#endif

class REvtBase
{
public:
  //! Wait for event.
  virtual void wait();
  //! Wait for event or time in msecs. 
  //! \return false on a timeout.
  virtual bool wait( int time );  
  virtual ~REvtBase();

  // Direct access not allowed due to combined event logic
  // possibility (i.e. atomic set the event and additional
  // info). 
  //HANDLE evt()  { return h; }

protected:

  REvtBase( HANDLE );

  HANDLE h;

};


// windows event wrapper
class REvent : public REvtBase
{
public:

  typedef REvtBase Parent;

  explicit REvent
	 (bool manual, //! manual reset
	  bool init = false //! initial state
		);
  ~REvent(){}
  virtual void set();
  virtual void reset();

};

/*
class SSemaphore : public REvtBase
{
public:

  typedef REvtBase Parent;

  explicit SSemaphore( int maxCount, int initCount = 0 );

  virtual void release( int count = 1 );

};
*/

size_t waitMultiple( HANDLE *, size_t count );

// include shutdown event also
//size_t waitMultipleSD( HANDLE *, size_t count );


#endif 
