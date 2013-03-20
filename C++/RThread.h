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
#include "RObjectWithStates.h"
#include <string>
#include <cstdatomic>
#include <thread>

/// An ancestor of all states of a thread.
class ThreadStateAxis : public StateAxis {};

/**
 * It is a base class for RThread. It contains the base
 * implementation which not depends on a particular physical thread.
 */
class RThreadBase
  : public SNotCopyable,
    public HasStringView,
    public RObjectWithStates<ThreadStateAxis>
  // TODO this class should has a complex state:
  // (currentState, waitCnt, exitRequested)
{
public:
  /// Contains a common thread execution code. It calls user-defined
  /// run(). It should be protected but is public for access from a
  /// derived RThread template.
  void _run ();

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
    return state_is(workingState);
  }

  // Overrides
  void outString (std::ostream& out) const;

  REvent& get_stop_event ()
  {
    return stopEvent;
  }

  const static StateMapPar new_states;
  typedef RState<RThreadBase, ThreadStateAxis, new_states> ThreadState;
  friend class RState<RThreadBase, ThreadStateAxis, new_states>;

  // States
  const static ThreadState readyState;
  const static ThreadState workingState;
  const static ThreadState terminatedState;
  const static ThreadState destroyedState;

  void state (ThreadState& state) const /* overrides */;

protected:

  typedef Logger<LOG::Thread> log;

  /// Mutex for concurrent access to this object.
  RMutex cs;

  void set_state_internal (const ThreadState& state) /* overrides */;

  void log_from_constructor ();

#if 0
  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );
#endif

  /// Start the thread procedure (called from _helper)


  /// It will be overrided with real thread procedure.
  virtual void run() = 0;

  virtual void start_impl () = 0;


  REvent stopEvent; //stop is requested

  /// A number of threads waiting termination
  /// of this thread.
  std::atomic<int> waitCnt; 

  /// Somebody has requested a termination.
  std::atomic<bool> exitRequested; 

  /*StateMap* get_state_map (ThreadState&) const
	 { return ThreadState::stateMap; }*/

private:
  //int _id;
  static std::atomic<bool> mainThreadCreated;
  static std::atomic<int> counter;

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
#if 0
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
	 const std::string& id,
	 REvent* extTerminated = 0) 
	 : RThreadBase (id, extTerminated) {}

  /* Access inside the thread */
  // Override it for a real thread job
  // but leave protected. It should not be called
  // directly!
  virtual void run() {}
  //!! release _current if it is needed
  virtual ~RThread();
#endif
};

template<>
class RThread<std::thread> : public RThreadBase
{
public:
	RThread(const std::string& id, REvent* extTerminated = 0)
	: RThreadBase(id, extTerminated) {}


protected:
	void start_impl () {
		th = new std::thread(&RThreadBase::_run, this);
	}

	std::thread* th;
};


void sSleep( int ms );

#endif
