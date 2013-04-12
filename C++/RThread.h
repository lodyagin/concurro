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
#if __GNUC_MINOR__< 6
#include <cstdatomic>
#else
#include <atomic>
#endif
#include <thread>

//! An ancestor of all states of a thread.
class ThreadAxis : public StateAxis {};

class RThreadImpl;

/**
 * It is a base class for RThread. It contains the base
 * implementation which not depends on a particular physical thread.
 */
class RThreadBase
  : public SNotCopyable,
    public HasStringView,
    public RObjectWithEvents<ThreadAxis>
{
  friend class RThreadImpl;

  DECLARE_EVENT(ThreadAxis, working);
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
  };

  const std::string universal_object_id;

  RThreadBase 
	 (const std::string& id,
	  RThreadImpl* impl_obj,
	  Event* extTerminated = 0
	  //!< If not null the thread will set this event at
     //! the exit of the run() method
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
  //void wait();  change with precise event waiting
  virtual void stop (); //!< try to stop implicitly

  bool is_running () const
  {
    return RAxis<ThreadAxis>::state_is
		(*this, workingState);
  }

  // Overrides
  void outString (std::ostream& out) const;

#if 0
  //! return the stopEvent
  Event& get_stop_event ()
  {
    return stopEvent;
  }
#endif

  DECLARE_STATES(ThreadAxis, ThreadState);
  DECLARE_STATE_CONST(ThreadState, ready);
  DECLARE_STATE_CONST(ThreadState, working);
  DECLARE_STATE_CONST(ThreadState, terminated);

  void state (ThreadState& state) const /* overrides */;

  DEFAULT_LOGGER(RThreadBase)

protected:

  bool destructor_delegate_is_called;

  //! stop is requested
  Event isStopRequested; 

  RThreadImpl* impl;

  RThreadBase(const ObjectCreationInfo&, const Par&);

  //! A real work of a destructor is here. All descendants
  //! must call it in the destructor. It waits the
  //! terminationEvent. 
  void destructor_delegate();

  void set_state_internal (const ThreadState& state);

  //void log_from_constructor ();

#if 0
  // It is ThreadProc, it simple calls _run ()
  // (Access inside the thread)
  static unsigned int __stdcall _helper( void * );
#endif

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

  // called from Windows
  // (Access inside the thread)
  //void _run();
};

class RThreadImpl : public SNotCopyable
{
  friend class RThreadBase;

protected:
  typedef Logger<RThreadImpl> log;

  RThreadBase* ctrl;

  RThreadImpl(RThreadBase* control)
	 : ctrl(control) {}

  //! Contains a common thread execution code. It calls
  //! user-defined run(). 
  void _run();

  //! It will be overrided with real thread procedure.
  virtual void run() = 0;
};

/**
 * Any kind thread-wrapper object. Thread is the real
 * thread class. Each RThread object must be member of a
 * ThreadRepository.
 */
template<class Thread, class Impl> class RThread {};

template<class Impl>
class RThread<std::thread, Impl> 
: public RThreadBase
{
public:
  typedef RThread<std::thread, Impl> This;

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
		rthread(0) 
	 {}

	 ~Par()
	 {
		rthreadStarted.wait();
	 }

	 RThreadBase* create_derivation
	   (const ObjectCreationInfo& oi) const
	 {
		assert(th); // get_id is called first
		rthread = new RThread<std::thread, Impl>
		  (oi, *this);
		rthread.impl = new RThreadImpl(rthread);
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
			(&RThread<std::thread, Impl>::Par::run0, 
			 const_cast<Par*>(this)));
		return th->native_handle();
	 }

	 std::unique_ptr<std::thread>&& move_thread()
	 { return std::move(th); }

	 void run0()
	 {
		rthreadCreated.wait();
		assert(rthread->impl);
		rthread->impl->_run();
		rthreadStarted.set();
		// <NB> `this' can be already destroyed 
		// at this point
	 }

  protected:
	 mutable std::unique_ptr<std::thread> th;
	 mutable Event rthreadCreated;
	 Event rthreadStarted;
	 mutable RThread<std::thread, Impl>* rthread;
	 RThreadImpl* impl;
  };

  RThread(const std::string& id, Event* extTerminated = 0)
	 : RThreadBase
	   (id, new RThreadImpl(this), extTerminated),
	 th(new std::thread
		 (&RThreadImpl::_run, impl)) {}

  ~RThread() 
  { 
	 destructor_delegate();
    //wait(); 
    th->join(); 
  }

  DEFAULT_LOGGER(This)

protected:
  //! It is for creation from ThreadRepository
  RThread(const ObjectCreationInfo& oi, const Par& par)
	 : RThreadBase(oi.objectId, par.extTerminated),
	 th(const_cast<Par&>(par).move_thread()) {}

  void start_impl () {}

  std::unique_ptr<std::thread> th;
};

#endif
