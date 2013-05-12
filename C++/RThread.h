// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file An unified wrapper over different type of threads
 * (i.e., QThread, posix thread etc.).
 */

#ifndef CONCURRO_RTHREAD_H_
#define CONCURRO_RTHREAD_H_

#include "SNotCopyable.h"
#include "RThread.h"
#include "RMutex.h"
#include "REvent.h"
#include "RState.h"
#include "SCommon.h"
#include "RObjectWithStates.h"
#include "Repository.h"
#include <string>
#if __GNUC_MINOR__< 6
#include <cstdatomic>
#else
#include <atomic>
#endif
#include <thread>

class ThreadAxis;

//! An ancestor of all states of a thread.
DECLARE_AXIS(ThreadAxis, StateAxis,
  {  "ready",         // after creation
	 "starting",      
	 "working",       
	 "terminated"
	 },
  {
    {"ready", "starting"},      // start ()

    {"starting", "working"},    
    // from a user-overrided run() method

    {"working", "terminated"},  
    // exit from a user-overrided run() 
	 // <NB> no ready->terminated, i.e.,
    // terminated means the run() was executed (once and
    // only once)
  }
);

/**
 * It is a base class for RThread. It contains the base
 * implementation which not depends on a particular
 * physical thread.
 */
class RThreadBase
  : public SNotCopyable,
    public HasStringView,
    public RObjectWithEvents<ThreadAxis>
{
  friend class ThreadAxis;

  DECLARE_EVENT(ThreadAxis, starting)
  DECLARE_EVENT(ThreadAxis, terminated)

public:

  struct Par
  {
	 Event* extTerminated;

    Par(Event* ext_terminated = 0) 
	   : extTerminated(ext_terminated) {}
	 virtual ~Par() {}
	 virtual RThreadBase* create_derivation
	   (const ObjectCreationInfo&) const = 0;
	 virtual RThreadBase* transform_object
	   (const RThreadBase*) const = 0;

	 std::string thread_name;
  };

  CompoundEvent is_terminal_state() const
  {
	 return is_terminated_event;
  }

  //! Contains a common thread execution code. It calls
  //! user-defined run(). It should be protected but is
  //! public for access from a derived RThread template.
  void _run ();

  //! Id of this thread in a repository
  const std::string universal_object_id;
  //! A pretty name for logging (if not empty)
  const std::string thread_name;

  RThreadBase 
	 (const std::string& id,
	  Event* extTerminated = 0
	  //!< If not null the thread will set this event at
     //! the exit of the run() method
	  );

  virtual ~RThreadBase ();

  std::string universal_id() const
  {
	 return universal_object_id;
  }

  std::string pretty_id() const
  {
	 return (thread_name.empty())
		? universal_object_id : thread_name;
  }

  /* 
  It is a group of functions 
  for access from a calling thread 
  */

  // TODO add std::call_once conditions. (? Also to dtr).

  void start();
  //void wait();  change with precise event waiting
  virtual void stop (); //!< try to stop implicitly

  bool is_running () const
  {
    return RAxis<ThreadAxis>::state_is
		(*this, RThreadBase::workingState);
  }

  // Overrides
  void outString (std::ostream& out) const;

  DECLARE_STATES(ThreadAxis, ThreadState);
  DECLARE_STATE_CONST(ThreadState, ready);
  DECLARE_STATE_CONST(ThreadState, starting);
  DECLARE_STATE_CONST(ThreadState, working);
  DECLARE_STATE_CONST(ThreadState, terminated);

  void state (ThreadState& state) const /* overrides */;

  DEFAULT_LOGGER(RThreadBase)

protected:

  struct LogParams {
	 static bool current;
  } log_params;

  bool destructor_delegate_is_called;

  //! stop is requested
  Event isStopRequested; 

  RThreadBase(const ObjectCreationInfo&, const Par&);

  //! A real work of a destructor is here. All descendants
  //! must call it in the destructor. It waits the
  //! terminationEvent. (Must be called from the most
  //! overrided destructor to prevent destroying of
  //! critical objects prior to _run() exit).
  void destroy();

  /* overrides */
  void set_state_internal (const ThreadState& state);

  void log_from_constructor ();

#if 0
  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );
#endif

  //! It will be overrided with real thread procedure.
  virtual void run() = 0;

  virtual void start_impl () = 0;

  //! A number of threads waiting termination
  //! of this thread.
  //TODO add this to (R)Event
  //std::atomic<int> waitCnt; 

private:
  //static std::atomic<int> counter;

  //thread terminate its processing
  //Event isTerminatedEvent; 
  Event* externalTerminated;
};

typedef RThreadBase::ThreadState RThreadState;

/*
  It can be started and stoped only once, like Java
  thread.  SException will be raised if we try to start
  several times.  */

/**
 * Any kind thread-wrapper object. Thread is the real
 * thread class. Each RThread object must be member of a
 * ThreadRepository.
 */
template<class Thread> class RThread {};

template<>
class RThread<std::thread> : public RThreadBase
{
public:

  typedef std::thread::native_handle_type ThreadId;
  typedef ThreadId Id;

  class Par;

  //! A RepositoryType to use with this RThread
  typedef RepositoryInterface<
	 RThread<std::thread>, Par, ThreadId > RepositoryType;

  class Par : public RThreadBase::Par
  {
  public:
    Par(Event* ext_terminated = 0)
		: RThreadBase::Par(ext_terminated),
		rthreadCreated(
		  "RThread<std::thread>::Par::rthreadCreated",
		  true, false),
		rthreadStarted(
		  "RThread<std::thread>::Par::rthreadStarted",
		  true, false),
		repository(0), th_id(0)
	 {}

	 ~Par()
	 {
		rthreadStarted.wait();
	 }

	 RThreadBase* transform_object
	   (const RThreadBase*) const
	 { 
		THROW_NOT_IMPLEMENTED; 
	 }

	 std::thread::native_handle_type get_id
		(ObjectCreationInfo& oi) const
	 {
		repository = dynamic_cast<RepositoryType*>
		  (oi.repository);
		SCHECK(repository);
		oi.objectCreated = &rthreadCreated;
		th = std::unique_ptr<std::thread>
		  (new std::thread
			(&RThread<std::thread>::Par::run0, 
			 const_cast<Par*>(this)));
		th_id = th->native_handle();
		return th_id;
	 }

	 std::unique_ptr<std::thread>&& move_thread()
	 { return std::move(th); }

	 void run0()
	 {
		assert(repository);
		rthreadCreated.wait();
		assert(th_id);
		RThread<std::thread>* rthread = repository
		  -> get_object_by_id(th_id);
		assert(rthread);
		rthreadStarted.set();
		// <NB> `this' can be already destroyed 
		// at this point
		rthread->_run();
	 }

  protected:
	 mutable std::unique_ptr<std::thread> th;
	 mutable Event rthreadCreated;
	 Event rthreadStarted;
	 //mutable RThread<std::thread>* rthread;
	 mutable RepositoryType* repository;
	 mutable std::thread::native_handle_type th_id;
  };

  RThread(const std::string& id, Event* extTerminated = 0)
	 : RThreadBase(id, extTerminated),
	 th(new std::thread
		 (&RThreadBase::_run, this)) {}

  ~RThread() 
  { 
	 // every descendant must call destroy() in its
	 // destructor. 
	 if (!destructor_delegate_is_called)
		THROW_PROGRAM_ERROR;

    th->join(); 
  }

  //! Create in the repository
  template<class Thread, class... Args>
  static Thread* create(Args&&... args);

  //! Destroy in the repository
  void remove();

  //! Return ptr to the current thread or nullptr if the
  //! current thread is not registered in a global
  //! RThread<std::thread> repository.
  static RThread<std::thread>* current();

  //! A pretty id of the current thread. 
  //! \see current()
  //! \see pretty_id() 
  static std::string current_pretty_id();

  //! Store an id of a current thread as a main thread
  static void this_is_main_thread()
  {
	 if (!(main_thread_id == std::thread::id()
			 || main_thread_id == std::this_thread::get_id()
			 ))
		THROW_PROGRAM_ERROR; // already set
	 main_thread_id = std::this_thread::get_id();
  }

  DEFAULT_LOGGER(RThread<std::thread>)

protected:
  //! It is for creation from ThreadRepository
  RThread(const ObjectCreationInfo& oi, const Par& par)
	 : RThreadBase(oi, par),
	 th(const_cast<Par&>(par).move_thread()) {}

  void start_impl () {}

  std::unique_ptr<std::thread> th;

  static std::thread::id main_thread_id;
};

#endif
