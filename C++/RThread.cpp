#include "StdAfx.h"
#include "RThread.h"
#include "SShutdown.h"
#include "Logging.h"
#include <assert.h>

// RThread states  ========================================

DEFINE_STATES(
  ThreadAxis,
  {  "ready",         // after creation
	 "working",       
	 "terminated"
	 },
  {
    {"ready", "working"},      // start ()
    {"working", "terminated"},  // exit from a user-overrided run()
    //<NB> no ready->terminated, i.e., terminated means the run()
    //was executed (once and only once)
  }
);

DEFINE_STATE_CONST(RThreadBase, ThreadState, ready);
DEFINE_STATE_CONST(RThreadBase, ThreadState, working);
DEFINE_STATE_CONST(RThreadBase, ThreadState, terminated);

DEFINE_EVENT(RThreadBase, ThreadAxis, working);
DEFINE_EVENT(RThreadBase, ThreadAxis, terminated);

RThreadBase::RThreadBase 
(const std::string& id,
 RThreadImpl* impl_obj,
 Event* extTerminated
)
  : 
    RObjectWithEvents<ThreadAxis> (readyState),
    universal_object_id (id),
	 //destructor_delegate_is_called(false),
    //waitCnt (0), 
    //isTerminatedEvent (false),
    isStopRequested
	   (SFORMAT("RThreadBase[id=" << id 
					<< "]::isStopRequested"), 
		 true, false),
	 impl(impl_obj),
	 externalTerminated (extTerminated)
{
  assert(impl);
  LOG_INFO (log, "New " << *this);
}

RThreadBase::RThreadBase
  (const ObjectCreationInfo& oi, const Par& p)
: 
    RObjectWithEvents<ThreadAxis> (readyState),
    universal_object_id (oi.objectId),
	 //destructor_delegate_is_called(false),
    isStopRequested 
	   (SFORMAT("RThreadBase[id=" << oi.objectId
					<< "]::isStopRequested"),
					true, false),
	 impl(0),
	 externalTerminated (p.extTerminated)
{
  LOG_INFO (log, "New " << *this);
}

void RThreadBase::start ()
{
  if (!impl) THROW_PROGRAM_ERROR;
  // impl must be set by the friend

  ThreadState::move_to (*this, workingState);
  start_impl ();
}

void RThreadBase::stop()
{
  isStopRequested.set ();
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

void RThreadBase::destructor_delegate()
{  
  isStopRequested.set();
  is_terminated_event.wait(*this);

  LOG_INFO(log, "Destroy " << *this);
}

RThreadBase::~RThreadBase()
{
  if (!destructor_delegate_is_called) 
    THROW_PROGRAM_ERROR;
  delete impl;
}


void RThreadImpl::_run()
{
  RThreadBase::is_working().wait(*ctrl);

  //TODO check run from current thread
  LOG_DEBUG(log, (*ctrl) << " started");
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

  LOG_DEBUG(log, *this << " is finished.");

  if (externalTerminated) 
    externalTerminated->set ();

  RThreadBase::ThreadState::move_to (*ctrl, terminatedState);
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


