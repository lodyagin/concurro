// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * An unified wrapper over different type of threads (i.e., QThread, posix thread etc.).
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
#include <cstdatomic>
#include <thread>

//! An ancestor of all states of a thread.
class ThreadAxis : public StateAxis {};

/**
 * It is a base class for RThread. It contains the base
 * implementation which not depends on a particular physical thread.
 */
class RThreadBase
  : public SNotCopyable,
    public HasStringView,
    public RObjectWithEvents<ThreadAxis>
  // TODO this class should has a complex state:
  // (currentState, waitCnt, exitRequested)
{
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
  };

  //! Contains a common thread execution code. It calls
  //! user-defined run(). It should be protected but is
  //! public for access from a derived RThread template.
  void _run ();

  const std::string universal_object_id;
  //const size_t num_id;

  RThreadBase 
	 (const std::string& id,
	  Event* extTerminated = 0
	  //!< If not null the thread will set this event at the exit of
     //! the run() method
	  );

  virtual ~RThreadBase ();

  std::string universal_id() const
  {
	 return universal_object_id;
  }

  //size_t id () const { return num_id; }

  /* 
  It is a group of functions 
  for access from a calling thread 
  */

  // TODO add std::call_once conditions. (? Also to dtr).

  void start();
  void wait();  // wait while thread finished
  virtual void stop (); //!< try to stop implicitly

  bool is_running () const
  {
//    RLOCK(cs);
    return RAxis<ThreadAxis>::state_is
		(*this, workingState);
  }

  // Overrides
  void outString (std::ostream& out) const;

#if 0
  Event& get_stop_event ()
  {
    return stopEvent;
  }
#endif

  DECLARE_STATES(ThreadAxis, ThreadState);
  DECLARE_STATE_CONST(ThreadState, ready);
  DECLARE_STATE_CONST(ThreadState, working);
  DECLARE_STATE_CONST(ThreadState, stop_requested);
  DECLARE_STATE_CONST(ThreadState, terminated);
  DECLARE_STATE_CONST(ThreadState, destroyed);

  void state (ThreadState& state) const /* overrides */;

  DEFAULT_LOGGER(RThreadBase)

protected:

  RThreadBase(const ObjectCreationInfo&, const Par&);

  //! Mutex for concurrent access to this object.
//  RMutex cs;

  void set_state_internal (const ThreadState& state) /* overrides */;

  void log_from_constructor ();

#if 0
  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );
#endif

  //! Start the thread procedure (called from _helper)


  //! It will be overrided with real thread procedure.
  //! It is not pure virtual to prevent situation of fast
  //! creation and destruction of an RThread object prior
  //! to _run() calls run().
  virtual void run() {};

  virtual void start_impl () {};


//  Event stopEvent; //stop is requested

  //! A number of threads waiting termination
  //! of this thread.
  std::atomic<int> waitCnt; 

  //! Somebody has requested termination. It is for
  //! checking from user-implemented run() method.
//  std::atomic<bool> exitRequested; 

private:
  static std::atomic<int> counter;

  //thread terminate its processing
  //Event isTerminatedEvent; 
  Event* externalTerminated;

  // called from Windows
  // (Access inside the thread)
  //void _run();
};

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

  class Par : public RThreadBase::Par
  {
  public:
    Par(Event* ext_terminated = 0)
		: RThreadBase::Par(ext_terminated),
		rthreadCreated(
		  "RThread<std::thread>::Par::rthreadCreated",
		  true, false),
		rthread(0) 
	 {}

	 RThreadBase* create_derivation
	   (const ObjectCreationInfo& oi) const
	 {
		assert(th); // get_id is called first
		rthread = new RThread<std::thread>(oi, *this);
		rthreadCreated.set();
      return rthread;
	 }

	 RThreadBase* transform_object
	   (const RThreadBase*) const
	 { 
		THROW_NOT_IMPLEMENTED; 
	 }

	 std::thread::native_handle_type get_id() const
	 {
		th = std::unique_ptr<std::thread>
		  (new std::thread
			(&RThread<std::thread>::Par::run0, 
			 const_cast<Par*>(this)));
		return th->native_handle();
	 }

	 std::unique_ptr<std::thread> move_thread()
	 { return std::move(th); }

	 void run0()
	 {
		rthreadCreated.wait();
		rthread->_run();
		// <NB> `this' can be already destroyed 
		// at this point
	 }

  protected:
	 mutable std::unique_ptr<std::thread> th;
	 mutable Event rthreadCreated;
	 mutable RThread<std::thread>* rthread;
  };

  RThread(const std::string& id, Event* extTerminated = 0)
	 : RThreadBase(id, extTerminated),
	 th(new std::thread
		 (&RThread<std::thread>::_run, this)) {}

  ~RThread() 
  { 
    wait(); 
    th->join(); 
  }

  DEFAULT_LOGGER(RThread<std::thread>)

protected:
  //! It is for creation from ThreadRepository
  RThread(const ObjectCreationInfo& oi, const Par& par)
	 : RThreadBase(oi.objectId, par.extTerminated),
	 th(const_cast<Par&>(par).move_thread()) {}

  void start_impl () {}

  std::unique_ptr<std::thread> th;
};

#endif
