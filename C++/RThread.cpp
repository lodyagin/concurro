#include "StdAfx.h"
#include "RThread.h"

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

#ifdef EVENT_IMPLEMENTED
RThreadBase::RThreadBase (SEvent* extTerminated) 
:
  RThread(),
  handle(0),
  isTerminatedEvent (false),
  stopEvent (true, false),
#ifndef EVENT_IMPLEMENTED
  waitCnt (0), 
#endif
  exitRequested (false),
  currentState ("ready"),
  externalTerminated (extTerminated)
{
  log_from_constructor ();
}
#endif

RThreadBase::RThreadBase (bool main)
  : 
#ifdef EVENT_IMPLEMENTED
    isTerminatedEvent (false),
    stopEvent (true, false),
#endif
#ifdef EVENT_IMPLEMENTED
    waitCnt (0), 
#endif
    exitRequested (false),
    currentState ("ready")
{
  if (main) {
    bool main_was_created = mainThreadCreated.exchange (true);
    if (main_was_created)
      throw SException (L"Only one thread with id = 0 can exist");
    _id = 0;
  }
  else _id = counter++;

  LOG4STRM_INFO
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

void RThreadBase::state (ThreadState& state) const
{
#ifdef MUTEX_IMPLEMENTED
  SMutex::Lock lock (cs);
#endif
  state = currentState;
}

void RThreadBase::start ()
{
#ifdef MUTEX_IMPLEMENTED
  SMutex::Lock lock(cs);
#endif
  ThreadState::check_moving_to (*this, workingState);

  start_impl ();
  // FIXME _current.set (0) on thread exit??

  ThreadState::move_to (*this, workingState);
}

#ifdef EVENT_IMPLEMENTED
void RThreadBase::wait()
{
  bool cntIncremented = false;
  try
  {
    { SMutex::Lock lock(cs);

      if (ThreadState::state_is (*this, terminatedState)
          || ThreadState::state_is (*this, readyState) // <NB>
        )
        // Shouldn't wait, it is already terminated
        return; 

      waitCnt++;
      cntIncremented = true;
    }

    LOG4STRM_DEBUG
      (Logging::Thread (),
       oss_ << "Wait a termination of ";
       outString (oss_);
       oss_ << " requested by ";
       RThread::current ().outString (oss_)
       );

    isTerminatedEvent.wait ();
    waitCnt--;
  }
  catch (...)
  {
     if (cntIncremented) waitCnt--;

     LOG4CXX_ERROR 
       (Logging::Root (), 
        "Unknown exception in RThread::wait");
  }
}
#endif

void RThreadBase::stop ()
{
#ifdef MUTEX_IMPLEMENTED
  SMutex::Lock lock(cs);
#endif
#ifdef EVENT_IMPLEMENTED
  stopEvent.set ();
#endif
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
#ifdef MUTEX_IMPLEMENTED
  SMutex::Lock lock (cs);
#endif
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
#ifdef MUTEX_IMPLEMENTED
    SMutex::Lock lock (cs);
#endif
    assert (!ThreadState::state_is (*this, destroyedState));
    needStop = ThreadState::state_is (*this, workingState)
      && !exitRequested;
    needWait = needStop || exitRequested;
  }

  if (needStop)
    stop ();

  if (needWait) 
    wait ();

  { 
#ifdef MUTEX_IMPLEMENTED
    SMutex::Lock lock (cs);
#endif
    ThreadState::move_to (*this, destroyedState);
  }

 LOG4STRM_INFO
    (Logging::Thread (),
     oss_ << "Destroy "; outString (oss_)
     );
}


void RThreadBase::_run()
{
   //TODO check run from current thread

   LOG4STRM_DEBUG
     (Logging::Thread (),
     outString (oss_); oss_ << " started");

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
      LOG4STRM_DEBUG
        (Logging::Thread (),
        oss_ << "Exception in thread: " << x.what ());
  }
  catch ( ... )
  {
    LOG4CXX_WARN
      (Logging::Thread (),
       "Unknown type of exception in thread.");
  }
  {
#ifdef MUTEX_IMPLEMENTED
    SMutex::Lock lock(cs);
#endif
    
    ThreadState::move_to (*this, terminatedState);
  }

   LOG4STRM_DEBUG
     (Logging::Thread (), 
      outString (oss_); oss_ << " finished"
      );

#ifdef EVENT_IMPLEMENTED
  if (externalTerminated) 
    externalTerminated->set ();
  isTerminatedEvent.set ();
#endif
}

#if 0
RThread & RThread::current()
{
  RThread * thread = reinterpret_cast<RThread*> 
    (RThread::get_current ());

  assert (thread); //FIXME check
  return *thread;
}
#endif

void RThreadBase::log_from_constructor ()
{
  LOG4STRM_INFO
    (Logging::Thread (),
     oss_ << "New "; outString (oss_) 
       );
}

