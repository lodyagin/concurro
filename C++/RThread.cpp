#include "StdAfx.h"
#include "RThread.h"
#include "SShutdown.h"
#include "Logging.h"
#include <assert.h>

// RThread states  ========================================

DEFINE_STATES(RThreadBase, ThreadStateAxis, ThreadState)
({  "ready",         // after creation
	 "working",       // it works
	 "stop_requested", // somebody called stop()
	 "terminated",    
	 "destroyed"    // to check a state in the destructor
	 },
  {
  {"ready", "working"},      // start ()

  // natural termination only
  {"working", "terminated"}, 

  // termination by a request
  {"working", "stop_requested"},
  {"stop_requested", "terminated"},

  // simultaneous stop requests are ignored
  {"stop_requested", "stop_requested"},

  {"terminated", "destroyed"},

  {"ready", "destroyed"}
  // can't be destroyed in other states
  }
  );

DEFINE_STATE_CONST(RThreadBase, ThreadState, ready);
DEFINE_STATE_CONST(RThreadBase, ThreadState, working);
DEFINE_STATE_CONST(RThreadBase, ThreadState, 
						 stop_requested);
DEFINE_STATE_CONST(RThreadBase, ThreadState, 
						 terminated);
DEFINE_STATE_CONST(RThreadBase, ThreadState, 
						 destroyed);

RThreadBase::RThreadBase 
(const std::string& id, 
 Event* extTerminated
)
  : 
    RObjectWithEvents<ThreadStateAxis> (readyState),
    universal_object_id (id),
    waitCnt (0), 
    isTerminatedEvent (false),
	 externalTerminated (extTerminated)
    //stopEvent (true, false),
    //exitRequested (false),
{
  LOG_INFO (log, "New " << *this);
}

RThreadBase::RThreadBase
  (const ObjectCreationInfo& oi, const Par& p)
: 
    RObjectWithEvents<ThreadStateAxis> (readyState),
    universal_object_id (oi.objectId),
    waitCnt (0), 
    isTerminatedEvent (false),
    //stopEvent (true, false),
    //exitRequested (false),
	 externalTerminated (p.extTerminated)
{
  LOG_INFO (log, "New " << *this);
}

#if 0
void RThreadBase::state (ThreadState& state) const
{
  RObjectWithStates<ThreadStateAxis>::state (state);
}

void RThreadBase::set_state_internal (const ThreadState& state)
{
  RLOCK(cs); // TODO: is it needed?
  RObjectWithStates<ThreadStateAxis>::set_state_internal 
	 (state);
}
#endif

void RThreadBase::start ()
{
//  RLOCK(cs);
  ThreadState::check_moving_to (*this, workingState);

  start_impl ();
  // FIXME _current.set (0) on thread exit??

  ThreadState::move_to (*this, workingState);
}

void RThreadBase::wait()
{
  bool cntIncremented = false;
  try
  {
	 if (ThreadState::state_in
		  (*this, {terminatedState, readyState}))
		// We shouldn't wait, it is already terminated
		return; 
	 
	 waitCnt++;
	 cntIncremented = true;

    LOG_DEBUG
      (Logger<LOG::Thread>,
       "Wait a termination of " << *this
       << " requested by " 
#ifdef CURRENT_NOT_IMPLEMENTED
		 << RThread::current ()
#endif
       );

	 REvent<ThreadStateAxis>("terminated").wait(*this);
    //isTerminatedEvent.wait ();
    waitCnt--;
  }
  catch (...)
  {
     if (cntIncremented) waitCnt--;

     LOG_ERROR(Logger<LOG::Root>, 
					"Unknown exception in RThread::wait");
  }
}

void RThreadBase::stop()
{
  ThreadState::move_to(*this, stop_requestedState);
/*  RLOCK(cs);
  stopEvent.set ();
  exitRequested = true;*/
}

#if 0
RThread* RThread::get_current ()
{
  return reinterpret_cast<RThread*> (_current.get ());
}

unsigned int __stdcall RThread::_helper( void * p )
{
  RThread * _this = reinterpret_cast<RThread *>(p);
  _current.set (_this);
  _this->_run();
  return 0;
}
#endif

void RThreadBase::outString (std::ostream& out) const
{
//  RLOCK(cs);
  out << "RThread(id = ["  
      << universal_object_id
      << "], this = " 
      << std::hex << (void *) this << std::dec
      << ", currentState = " 
		<< RState<ThreadStateAxis>(*this) << ')';
}

std::atomic<int> RThreadBase::counter (0);
 

// For proper destroying in concurrent environment
// 1) nobody may hold RThread* and descendants (even temporary!), // only access through Repository is allowed
// FIXME !!!

RThreadBase::~RThreadBase()
{
  bool needWait = true;
  bool needStop = true;
  { 
#ifdef STATE_LOCKING
	 auto st = ThreadState::lock_state(*this);
#else
	 auto st = ThreadState::state(*this);
#endif
	 SCHECK(st != destroyedState);
    needStop = st == workingState;
    needWait = needStop || st == stop_requestedState;
  }

  if (needStop)
    stop ();

  if (needWait) 
    wait ();

  ThreadState::move_to (*this, destroyedState);

  LOG_INFO(log, "Destroy " << *this);
}


void RThreadBase::_run()
{
   //TODO check run from current thread
  LOG_DEBUG(log, *this << " started");
  try
  {
    run();
  }
  catch ( XShuttingDown & )
  {
    // Do nothing - execution is finished
  }
  catch ( std::exception & x )
  {
	 LOG_DEBUG(log, "Exception in thread: " 
						 << x.what ());
  }
  catch ( ... )
  {
    LOG_WARN(log, 
		"Unknown type of exception in the thread.");
  }

  ThreadState::move_to (*this, terminatedState);

  LOG_DEBUG(log, *this << " is finished.");

  if (externalTerminated) 
    externalTerminated->set ();
  //isTerminatedEvent.set ();
}

void RThreadBase::log_from_constructor ()
{
  LOG_INFO(log, "New " << *this);
}

#if 0

template<class Thread>
RThread<Thread> & RThread<Thread>::current()
{
  RThread * thread = reinterpret_cast<RThread*> 
    (RThread::get_current ());

  SCHECK(thread); //FIXME check
  return *thread;
}
#endif


