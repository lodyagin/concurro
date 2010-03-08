#ifndef __STHREAD_H
#define __STHREAD_H

#include "SMutex.h"
#include "SEvent.h"
#include "SNotCopyable.h"
#include "StateMap.h"

class SThreadBase
  : public SNotCopyable,
    public HasStringView
{
public:
  enum Main { main };
  enum External { external };

  class Tls; // storage for data separated by threads

  SThreadBase ();
  explicit SThreadBase (Main);
  explicit SThreadBase (External);
  ~SThreadBase ();
  int id() const { return _id; }

  // Overrides
  void outString (std::ostream& out) const;

protected:
  
  static SThreadBase* get_current ();

  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );

  // the thread procedure (called from _helper)
  virtual void _run () = 0;

private:

  int _id;
  static Tls _current;

  static bool mainThreadCreated;

  static SMTCounter counter;
};

/*
  It can be started and stoped only once, like Java thread.
  SException will be raised if we try to start several times.
*/

class ThreadStateSpace {};

class SThread : public SThreadBase
{
public:

  // TODO move all mechanics somewhere 
  // (base class or templ parameter)
  class ThreadState 
    : public UniversalState, // TODO protected
      public ThreadStateSpace
  {
    friend SThread;
  public:
    ThreadState (const char* name);

    // TODO base mechanics
    template<class Object>
    static void check_moving_to
      (const Object& obj, const ThreadState& to)
    {
      ThreadState from = to;
      obj.state (from);
      obj.get_state_map (from) -> check_transition
        (from, to);
    }

    // TODO base mechanics
    template<class Object>
    static void move_to
      (Object& obj, const ThreadState& to)
    {
      check_moving_to (obj, to);
      obj.set_state_internal (to);
      // FIXME add logging
    }

    // TODO base mechanics
    template<class Object>
    static bool state_is
      (const Object& obj, const ThreadState& st)
    {
      ThreadState current = st;
      obj.state (current);
      return current.state_idx == st.state_idx;
    }

    // TODO base mechanics
    std::string name () const
    {
      return stateMap->get_state_name (*this);
    }

  protected:
    static StateMap* stateMap;
  };

  // States
  static ThreadState readyState;
  static ThreadState workingState;
  static ThreadState terminatedState;
  static ThreadState destroyedState;

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

  // Return the pointer to the SThread object
  // for the current thread.
  // TODO UT on thread not created by SThread.
  static SThread & current();
  // Overrides
  void outString (std::ostream& out) const;

  SEvent& get_stop_event ()
  {
    return stopEvent;
  }

  void state (ThreadState& state) const;

protected:

  void set_state_internal (const ThreadState& state)
  {
    currentState = state;
  }

  // Thread must be always allocated dinamically
  SThread (SEvent* extTerminated = 0);

  explicit SThread( Main );  // must be one main thread
  explicit SThread( External );  // must be one main thread

  void log_from_constructor ();

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

  StateMap* get_state_map (ThreadState&) const
  { return ThreadState::stateMap; }

private:

  static const State2Idx allStates[];
  static const StateTransition allTrans[];

  // this class has a complex state:
  // (currentState, waitCnt, exitRequested)
  ThreadState currentState;

  //thread terminate its processing
  SEvent isTerminatedEvent; 
  SEvent* externalTerminated;

  // called from Windows
  // (Access inside the thread)
  void _run();

  HANDLE handle;
};



// thread local storage
class SThreadBase::Tls
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
