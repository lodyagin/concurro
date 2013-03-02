/**
 * @file
 * An unified wrapper over different type of threads (i.e., QThread, posix thread etc.).
 */

#ifndef RTHREADWRAPPER_H_
#define RTHREADWRAPPER_H_

#include "SNotCopyable.h"
#include "StateMap.h"
#include "RThread.h"
#include "RMutex.h"
#include "REvent.h"
#include "SCommon.h"
#include <string>
#include <cstdatomic>

class ThreadStateSpace {};

/**
 * It is a base class for RThread. It contains the base
 * implementation which not depends on a particular physical thread.
 */
class RThreadBase
  : public SNotCopyable,
    public HasStringView
{
public:
  /// It is mandatory for any repository member. 
  // Is always > 0. As a special
  /// meaning the thread with 1 id is a main thread
  const std::string universal_object_id;
  const size_t num_id;

  RThreadBase 
	 (const std::string& id,
	  ///< it comes from Repository::create_object. id == 1
	  /// for the main thread
	  REvent* extTerminated = 0
	  ///< If not null the thread will set this event at the exit of
     /// the run() method
	  );

  virtual ~RThreadBase ();
  size_t id () const { return num_id; }

  /* 
  It is a group of functions 
  for access from a calling thread 
  */
  void start();
  void wait();  // wait while thread finished
  virtual void stop (); ///< try to stop implicitly

  bool is_running () const
  {
    RLOCK(cs);
    return currentState == workingState;
  }

  // Overrides
  void outString (std::ostream& out) const;

  REvent& get_stop_event ()
  {
    return stopEvent;
  }

  // the state stuff
  class ThreadState 
    : public UniversalState, // TODO protected
      public ThreadStateSpace
  {
    friend class RThreadBase;
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

  void state (ThreadState& state) const;

protected:

  typedef Logger<LOG::Thread> logger;

  /// Mutex for concurrent access to this object.
  RMutex cs;

  void set_state_internal (const ThreadState& state)
  {
    currentState = state;
  }

  void log_from_constructor ();

#if 0
  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );
#endif

  /// Start the thread procedure (called from _helper)
  void _run ();

  /// It will be overrided with real thread procedure.
  virtual void run() = 0;

  virtual void start_impl () = 0;


  REvent stopEvent; //stop is requested

  /// A number of threads waiting termination
  /// of this thread.
  std::atomic<int> waitCnt; 

  /// Somebody has requested a termination.
  std::atomic<bool> exitRequested; 

  StateMap* get_state_map (ThreadState&) const
  { return ThreadState::stateMap; }

private:
  //int _id;
  static std::atomic<bool> mainThreadCreated;
  static std::atomic<int> counter;

  static const State2Idx allStates[];
  static const StateTransition allTrans[];

  // this class has a complex state:
  // (currentState, waitCnt, exitRequested)
  ThreadState currentState;

  //thread terminate its processing
  REvent isTerminatedEvent; 
  REvent* externalTerminated;

  // called from Windows
  // (Access inside the thread)
  //void _run();
};

/*
  It can be started and stoped only once, like Java thread.
  SException will be raised if we try to start several times.
*/

/**
 * Any kind thread-wrapper object. Thread is the real thread
 * class. Each RThread object must be member of a ThreadRepository.
 */
template<class Thread>
class RThread
  : public RThreadBase,
    protected Thread
{
public:

  /// Create a new thread. main == true will crate the main
  /// thread. One and only one main thread can be created.
//  static RThread* create (bool main = false) {
//    return new RThread (main);
//  }

  /// Return the pointer to the RThread object
  /// for the current thread.
  // TODO UT on thread not created by RThread.
  static RThread<Thread>& current();

  // Overrides
  void outString (std::ostream& out) const;

protected:
  
  /// Create a new thread. main == true will crate the main
  /// thread. One and only one main thread can be created.
  // !! set _current if it is needed
  explicit RThread (
	 size_t id,
	 REvent* extTerminated = 0) 
	 : RThreadBase (id, extTerminated) {}

  /* Access inside the thread */
  // Override it for a real thread job
  // but leave protected. It should not be called
  // directly!
  virtual void run() {}
  //!! release _current if it is needed
  virtual ~RThread();
};

void sSleep( int ms );


#endif
