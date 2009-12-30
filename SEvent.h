#ifndef __SEVENT_H
#define __SEVENT_H

class SEvtBase
{
public:

  ~SEvtBase();

  void wait();
  bool wait( int time );  // false on timeout; time in millisecs

  HANDLE evt()  { return h; }

protected:

  SEvtBase( HANDLE );

  HANDLE h;

};


// windows event wrapper
class SEvent : public SEvtBase
{
public:

  typedef SEvtBase Parent;

  explicit SEvent( bool manual, bool init = false );

  void set();
  void reset();

};


class SSemaphore : public SEvtBase
{
public:

  typedef SEvtBase Parent;

  explicit SSemaphore( int maxCount, int initCount = 0 );

  void release( int count = 1 );

};


size_t waitMultiple( HANDLE *, size_t count );

// include shutdown event also
size_t waitMultipleSD( HANDLE *, size_t count );


#endif  // __SEVENT_H
