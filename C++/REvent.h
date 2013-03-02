#ifndef REVENT_H
#define REVENT_H

#ifndef _WIN32
#define WFMO
#include "pevents.h"
typedef neosmart::neosmart_event_t HANDLE;
#endif

class REvtBase
{
public:
  void wait();
  bool wait( int time );  // false on timeout; time in millisecs
  virtual ~REvtBase();
  HANDLE evt()  { return h; }

protected:

  REvtBase( HANDLE );

  HANDLE h;

};


// windows event wrapper
class REvent : public REvtBase
{
public:

  typedef REvtBase Parent;

  explicit REvent( bool manual, bool init = false );
  ~REvent(){}
  void set();
  void reset();

};

/*
class SSemaphore : public REvtBase
{
public:

  typedef REvtBase Parent;

  explicit SSemaphore( int maxCount, int initCount = 0 );

  void release( int count = 1 );

};
*/

size_t waitMultiple( HANDLE *, size_t count );

// include shutdown event also
//size_t waitMultipleSD( HANDLE *, size_t count );


#endif 
