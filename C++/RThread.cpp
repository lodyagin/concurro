#include "StdAfx.h"
#include "RThread.h"
#include "SShutdown.h"
#include "Logging.h"
#include <assert.h>

// RThread states  ========================================

const StateMapPar RThreadBase::new_states
({  "ready",         // after creation
	 "working",       // it works
	 "terminated",    
	 "destroyed"    // to check a state in the destructor
	 },
  {
  {"ready", "working"},      // start ()

  // a) natural termination
  // b) termination by request
  {"working", "terminated"},      

  {"terminated", "destroyed"},

  {"ready", "destroyed"}
  // can't be destroyed in other states
  }
  );

const RThreadBase::ThreadState RThreadBase::readyState("ready");
const RThreadBase::ThreadState RThreadBase::workingState("working");
const RThreadBase::ThreadState RThreadBase::terminatedState("terminated");
const RThreadBase::ThreadState RThreadBase::destroyedState("destroyed");

RThreadBase::RThreadBase 
(const std::string& id, 
 REvent* extTerminated
)
  : 
    universal_object_id (id),
	 num_id (fromString<size_t>(id)),
    isTerminatedEvent (false),
    stopEvent (true, false),
    waitCnt (0), 
    exitRequested (false),
    RObjectWithStates<ThreadStateAxis> (readyState),
	 externalTerminated (extTerminated),
	 cs(SFORMAT("RThreadBase with id=["<<id<<"]"))
{
  if (num_id == 0) {
	 THROW_PROGRAM_ERROR;
  }
  else if (num_id == 1) {
    bool main_was_created = mainThreadCreated.exchange (true);
    if (main_was_created)
      throw SException (_T"Only one thread with id = 1 can exist");
  }
  LOG_INFO (log, "New " << *this);
}

RThreadBase::RThreadBase
  (const ObjectCreationInfo& oi, const Par& p)
: 
    universal_object_id (oi.objectId),
	 num_id (fromString<size_t>(oi.objectId)),
    isTerminatedEvent (false),
    stopEvent (true, false),
    waitCnt (0), 
    exitRequested (false),
    RObjectWithStates<ThreadStateAxis> (readyState),
	 externalTerminated (p.extTerminated),
	 cs(SFORMAT("RThreadBase with id=["<<oi.objectId<<"]"))
{
  if (num_id == 0) {
	 THROW_PROGRAM_ERROR;
  }
  else if (num_id == 1) {
    bool main_was_created = mainThreadCreated.exchange (true);
    if (main_was_created)
      throw SException (_T"Only one thread with id = 1 can exist");
  }
  LOG_INFO (log, "New " << *this);
}

void RThreadBase::state (ThreadState& state) const
{
  RLOCK(cs); // TODO: is it needed?
  RObjectWithStates<ThreadStateAxis>::state (state);
}

void RThreadBase::set_state_internal (const ThreadState& state)
{
  RLOCK(cs); // TODO: is it needed?
  RObjectWithStates<ThreadStateAxis>::set_state_internal (state);
}

void RThreadBase::start ()
{
  RLOCK(cs);
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
    { RLOCK(cs);

      if (ThreadState::state_is (*this, terminatedState)
          || ThreadState::state_is (*this, readyState) // <NB>
        )
        // Shouldn't wait, it is already terminated
        return; 

      waitCnt++;
      cntIncremented = true;
    }

    LOG_DEBUG
      (Logger<LOG::Thread>,
       "Wait a termination of " << *this
       << " requested by " 
#ifdef CURRENT_NOT_IMPLEMENTED
		 << RThread::current ()
#endif
       );

    isTerminatedEvent.wait ();
    waitCnt--;
  }
  catch (...)
  {
     if (cntIncremented) waitCnt--;

     LOG_ERROR(Logger<LOG::Root>, "Unknown exception in RThread::wait");
  }
}

void RThreadBase::stop ()
{
  RLOCK(cs);
  stopEvent.set ();
  exitRequested = true;
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
  RLOCK(cs);
  out << "RThread(id = "  
      << id ()
      << ", this = " 
      << std::hex << (void *) this << std::dec
      << ", currentState = " 
      << currentState.name ()
      << ')';
}

std::atomic<int> RThreadBase::counter (0);
std::atomic<bool> RThreadBase::mainThreadCreated (false);
 

// For proper destroying in concurrent environment
// 1) nobody may hold RThread* and descendants (even temporary!), // only access through Repository is allowed
// FIXME !!!

RThreadBase::~RThreadBase()
{
  bool needWait = true;
  bool needStop = true;
  { 
    RLOCK(cs);
    SCHECK(!ThreadState::state_is(*this, destroyedState));
    needStop = ThreadState::state_is (*this, workingState)
      && !exitRequested;
    needWait = needStop || exitRequested;
  }

  if (needStop)
    stop ();

  if (needWait) 
    wait ();

  { 
    RLOCK(cs);
    ThreadState::move_to (*this, destroyedState);
  }
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
  {
    RLOCK(cs);
    
    ThreadState::move_to (*this, terminatedState);
  }

  LOG_DEBUG(log, *this << " is finished.");

  if (externalTerminated) 
    externalTerminated->set ();
  isTerminatedEvent.set ();
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


