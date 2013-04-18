// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * An unified wrapper over different type of threads (i.e., QThread, posix thread etc.).
 */

#include "StdAfx.h"
#include "RThread.h"
#include "SShutdown.h"
#include "Logging.h"
#include <assert.h>

// RThread states  ========================================

DEFINE_STATES(
  ThreadAxis,
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

DEFINE_STATE_CONST(RThreadBase, ThreadState, ready);
DEFINE_STATE_CONST(RThreadBase, ThreadState, starting);
DEFINE_STATE_CONST(RThreadBase, ThreadState, working);
DEFINE_STATE_CONST(RThreadBase, ThreadState, terminated);

//DEFINE_EVENT(RThreadBase, ThreadAxis, starting);
//DEFINE_EVENT(RThreadBase, ThreadAxis, terminated);

RThreadBase::RThreadBase 
(const std::string& id, 
 Event* extTerminated
)
  : 
    RObjectWithEvents<ThreadAxis> (readyState),
	 CONSTRUCT_EVENT(starting),
	 CONSTRUCT_EVENT(terminated),
    universal_object_id (id),
	 destructor_delegate_is_called(false),
    isStopRequested
	   (SFORMAT("RThreadBase[id=" << id 
					<< "]::isStopRequested"), 
		 true, false),
	 externalTerminated (extTerminated)
{
  LOG_DEBUG (log, "thread " << universal_object_id << "> created");
}

RThreadBase::RThreadBase
  (const ObjectCreationInfo& oi, const Par& p)
: 
    RObjectWithEvents<ThreadAxis> (readyState),
	 CONSTRUCT_EVENT(starting),
	 CONSTRUCT_EVENT(terminated),
    universal_object_id (oi.objectId),
	 destructor_delegate_is_called(false),
    isStopRequested 
	   (SFORMAT("RThreadBase[id=" << oi.objectId
					<< "]::isStopRequested"),
					true, false),
	 externalTerminated (p.extTerminated)
{
  LOG_DEBUG (log, "thread " << oi.objectId << "> created");
}

void RThreadBase::start ()
{
  ThreadState::move_to (*this, startingState);
  start_impl ();
}

void RThreadBase::stop()
{
  isStopRequested.set ();
}

RThread<std::thread>* RThread<std::thread>::current ()
{
#ifdef _WIN32
  return reinterpret_cast<RThread*> (_current.get ());
#else
  
#endif
}

#if 0
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
  out << "RThread(id = ["  
      << universal_object_id
      << "], this = " 
      << std::hex << (void *) this << std::dec
      << ", currentState = " 
		<< RState<ThreadAxis>(*this) << ')';
}

//std::atomic<int> RThreadBase::counter (0);
 

// For proper destroying in concurrent environment
// 1) nobody may hold RThread* and descendants (even temporary!), // only access through Repository is allowed
// FIXME !!!

RThreadBase::~RThreadBase()
{
  if (!destructor_delegate_is_called)
	 THROW_PROGRAM_ERROR;
  // must be called from desc. destructors
}


void RThreadBase::destroy()
{
  if (destructor_delegate_is_called) 
    return;

  isStopRequested.set();
  is_terminated_event.wait();

  LOG_DEBUG (log, "thread " << universal_object_id << "> destroyed");
  destructor_delegate_is_called = true;
}


void RThreadBase::_run()
{
  is_starting_event.wait();

  //TODO check run from current thread
  LOG_DEBUG (log, "thread " << universal_object_id << "> started");
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
	 LOG_WARN(log, "Exception in thread: " 
						 << x.what ());
  }
  catch ( ... )
  {
    LOG_WARN(log, 
		"Unknown type of exception in the thread.");
  }

  LOG_DEBUG (log, "thread " << universal_object_id << "> finished");

  if (externalTerminated) 
    externalTerminated->set ();

  ThreadState::move_to (*this, terminatedState);
  // no code after that (destructor can be called)
}

#if 0

void RThreadBase::log_from_constructor ()
{
  LOG_INFO(log, "New " << *this);
}

template<class Thread>
RThread<Thread> & RThread<Thread>::current()
{
  RThread * thread = reinterpret_cast<RThread*> 
    (RThread::get_current ());

  SCHECK(thread); //FIXME check
  return *thread;
}
#endif


