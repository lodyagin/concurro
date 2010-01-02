#ifndef __STHREAD_H
#define __STHREAD_H

#include "SMutex.h"
#include "SEvent.h"
#include "SNotCopyable.h"
#include "Logging.h"
#include "StateMap.h"

/*
  It can be started and stoped only once, like Java thread.
  SException will be raised if we try to start several times.
*/
class SThread : public SNotCopyable
{
    static Logging m_Logging;
public:

  class Tls; // storage for data separated by threads

  enum Main { main };
  enum External { external };

  /* 
  It is a group of functions 
  for access from a calling thread 
  */
  SThread();
  explicit SThread( Main );  // must be one main thread
  explicit SThread( External );  // must be one main thread
  virtual ~SThread();
  void start();
  void wait();  // wait while thread finished
  void stop (); // try to stop implicitly
  //bool isTerminated () const { return exit; }

  /* For access from both. */
  // Return the pointer to the SThread object
  // for the current thread.
  // TODO UT on thread not created by SThread.
  static SThread & current();
  int id() const { return _id; }

  /* For access inside the thread */
  // return 'true' if stop is requested
  bool is_stop_requested ();

protected:
  // Override it for a real thread job
  // but leave protected. It should not be called
  // directly!
  // Access inside the thread
  virtual void run() {}

private:

  /* A state machine */
  StateMap* stateMap;

  UniversalState readyState;
  UniversalState workingState;
  UniversalState exitRState;
  UniversalState waitingState;
  UniversalState selfTerminatedState;
  UniversalState terminatedByRState;
  UniversalState destroyedState;

  UniversalState currentState;

  SEvent isTerminatedEvent;

  void check_moving_to (const UniversalState& to);

  void move_to (const UniversalState& to);

  static const State2Idx allStates[];
  static const StateTransition allTrans[];

  void initializeStates ();
  /* end of the state machine */

  // called from Windows
  // (Access inside the thread)
  void _run();

  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static DWORD WINAPI _helper( void * );

  HANDLE handle;
  int _id;

  static SMTCounter counter;
  static Tls _current;

};



// thread local storage
class SThread::Tls
{
public:

  Tls();
  ~Tls();

  // Each thread has its own slot for value.
  void * get();
  void set( void * );

private:

  DWORD idx;

};


void sSleep( int ms );


#endif  // __STHREAD_H
