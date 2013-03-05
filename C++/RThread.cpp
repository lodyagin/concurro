#include "StdAfx.h"
#include "RThread.h"
#include "SShutdown.h"
#include "Logging.h"
#include <assert.h>

// RThread states  ========================================

const State2Idx RThreadBase::allStates[] =
{
  {1, "ready"},         // after creation
  {2, "working"},       // it works
  {3, "terminated"},    
  {4, "destroyed"},    // to check a state in the destructor
  {0, 0}
};

const StateTransition RThreadBase::allTrans[] =
{
  {"ready", "working"},      // start ()

  // a) natural termination
  // b) termination by request
  {"working", "terminated"},      

  {"terminated", "destroyed"},

  {"ready", "destroyed"},
  // can't be destroyed in other states

  {0, 0}

};

StateMap* RThreadBase::ThreadState::stateMap = 0;

RThreadBase::ThreadState::ThreadState (const char* name)
{
  assert (name);

  if (!stateMap)
    stateMap = new StateMap(allStates, allTrans);

  *((UniversalState*) this) = stateMap->create_state (name);
}


RThreadBase::ThreadState RThreadBase::readyState("ready");
RThreadBase::ThreadState RThreadBase::workingState("working");
RThreadBase::ThreadState RThreadBase::terminatedState("terminated");
RThreadBase::ThreadState RThreadBase::destroyedState("destroyed");

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
    currentState ("ready"),
	 externalTerminated (extTerminated)
{
  if (num_id == 0) {
	 THROW_PROGRAM_ERROR;
  }
  else if (num_id == 1) {
    bool main_was_created = mainThreadCreated.exchange (true);
    if (main_was_created)
      throw SException (_T"Only one thread with id = 1 can exist");
  }
  LOG_INFO (logger, "New " << *this);
}

void RThreadBase::state (ThreadState& state) const
{
  RLOCK(cs);
  state = currentState;
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
  LOG_INFO(logger, "Destroy " << *this);
}


void RThreadBase::_run()
{
   //TODO check run from current thread
  LOG_DEBUG(logger, *this << " started");
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
	 LOG_DEBUG(logger, "Exception in thread: " 
						 << x.what ());
  }
  catch ( ... )
  {
    LOG_WARN(logger, 
		"Unknown type of exception in the thread.");
  }
  {
    RLOCK(cs);
    
    ThreadState::move_to (*this, terminatedState);
  }

  LOG_DEBUG(logger, *this << " is finished.");

  if (externalTerminated) 
    externalTerminated->set ();
  isTerminatedEvent.set ();
}

void RThreadBase::log_from_constructor ()
{
  LOG_INFO(logger, "New " << *this);
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


