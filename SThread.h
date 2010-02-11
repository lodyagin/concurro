#ifndef __STHREAD_H
#define __STHREAD_H

#include "SMutex.h"
#include "SEvent.h"
#include "SNotCopyable.h"
#include "StateMap.h"

/*
  It can be started and stoped only once, like Java thread.
  SException will be raised if we try to start several times.
*/
class SThread 
  : public SNotCopyable,
    public HasStringView
{
public:

  class Tls; // storage for data separated by threads

  enum Main { main };
  enum External { external };

  // Create new thread.
  // It will be destroyed when the thread
  // function will exit if selfDestroing = true.
  // Use it instead of constructor.
  static SThread* create ();
  static SThread* create (Main);
  static SThread* create (External);

  const bool selfDestroing;

  /* 
  It is a group of functions 
  for access from a calling thread 
  */
  virtual void start();
  virtual void wait();  // wait while thread finished
  virtual void stop (); // try to stop implicitly
  //bool isTerminated () const { return exit; }

  bool is_running () const
  {
    SMutex::Lock lock (cs);
    return currentState == workingState;
  }

  /* For access from both. */
  // Return the pointer to the SThread object
  // for the current thread.
  // TODO UT on thread not created by SThread.
  static SThread & current();
  int id() const { return _id; }
  // Overrides
  void outString (std::ostream& out) const;

  SEvent& get_stop_event ()
  {
    return stopEvent;
  }

protected:
  // Thread must be always allocated dinamically
  SThread();
  explicit SThread( Main );  // must be one main thread
  explicit SThread( External );  // must be one main thread

  /* Access inside the thread */
  // Override it for a real thread job
  // but leave protected. It should not be called
  // directly!
  virtual void run() {}
  virtual ~SThread();

  SEvent stopEvent; //stop is requested

  SMutex cs;

  // number of threads waiting termination
  // of this thread
  volatile int waitCnt; 

  // somebody has requested a termination
  volatile bool exitRequested; 

  bool state_is (const UniversalState& state) const
  {
    return stateMap->is_equal (currentState, state);
  }

private:

  /* A state machine */
  static StateMap* stateMap;

  static UniversalState readyState;
  static UniversalState workingState;
  static UniversalState selfTerminatedState;
  static UniversalState terminatedByRState;
  static UniversalState destroyedState;

  static const State2Idx allStates[];
  static const StateTransition allTrans[];
public:
  static void initializeStates ();
private:
  // this class has a complex state:
  // (currentState, isWaited, exitRequested)
  UniversalState currentState;

  //thread terminate its processing
  SEvent isTerminatedEvent; 

  void check_moving_to (const UniversalState& to);

  void move_to (const UniversalState& to);

  /* end of the state machine */

  // called from Windows
  // (Access inside the thread)
  void _run();

  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );

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
