#include "stdafx.h"
#include "SThread.h"
#include <objbase.h>


using namespace std;

const State2Idx SThread::allStates[] =
{
  {1, "ready"},         // after creation
  {2, "working"},       // it works
  {3, "exitRequested"}, // it works by stop already requested
  {4, "terminated"},    
  {5, "destroyed"},    // to check a state in the destructor
  {0, 0}
};

const StateTransition SThread::allTrans[] =
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

// SThreadBase  ==========================================================

SThreadBase::SThreadBase ()
: _id ( counter++ )
{
}

SThreadBase::SThreadBase (External)
: _id ( - (counter++) )
{
  _current.set (this);
}

SThreadBase::SThreadBase (Main)
: _id (0)
{
  if (mainThreadCreated)
    throw SException 
      (L"Only one thread with id = 0 can exist");
  mainThreadCreated = true; //FIXME concurrency
  _current.set (this);
}

SThreadBase::~SThreadBase ()
{
  if ( id () <= 0 ) 
    _current.set (0);
}

SThreadBase* SThreadBase::get_current ()
{
  return reinterpret_cast<SThreadBase*> (_current.get ());
}

unsigned int __stdcall SThreadBase::_helper( void * p )
{
  SThreadBase * _this = reinterpret_cast<SThreadBase *>(p);
  _current.set (_this);
  _this->_run();
  return 0;
}

void SThreadBase::outString (std::ostream& out) const
{
  out << "SThreadBase(id = "  
      << id ()
      << ", this = " 
      << std::hex << (void *) this << std::dec
      << ')';
}

SMTCounter SThreadBase::counter(0);
SThreadBase::Tls SThreadBase::_current;
bool SThreadBase::mainThreadCreated = false;
 

// SThread  ==========================================================

StateMap* SThread::ThreadState::stateMap = 0;

SThread::ThreadState::ThreadState (const char* name)
{
  assert (name);

  if (!stateMap)
    stateMap = new StateMap(allStates, allTrans);

  *((UniversalState*) this) = stateMap->create_state (name);
}


SThread::ThreadState SThread::readyState("ready");
SThread::ThreadState SThread::workingState("working");
SThread::ThreadState SThread::terminatedState("terminated");
SThread::ThreadState SThread::destroyedState("destroyed");

SThread* SThread::create ()
{
  return new SThread ();
}

SThread* SThread::create (Main p)
{
  return new SThread (p);
}

SThread* SThread::create (External p)
{
  return new SThread (p);
}

SThread::SThread(SEvent* extTerminated) :
  SThreadBase(),
  handle(0),
  isTerminatedEvent (false),
  selfDestroing (true),
  stopEvent (true, false),
  waitCnt (0), exitRequested (false),
  currentState ("ready"),
  externalTerminated (extTerminated)
{

  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

SThread::SThread( Main ) :
  SThreadBase ( main ),
  handle(0),
  isTerminatedEvent (false),
  selfDestroing (false),
  stopEvent (true, false),
  waitCnt (0), exitRequested (false),
  currentState ("ready")
{
  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

SThread::SThread( External ) :
  SThreadBase (external),
  handle(0),
  isTerminatedEvent (false),
  selfDestroing (true),
  stopEvent (true, false),
  waitCnt (0), exitRequested (false),
  currentState ("ready")
{
  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

void SThread::state (ThreadState& state) const
{
  state = currentState;
}

SThread::~SThread()
{
  bool needWait = true;
  { 
    SMutex::Lock lock (cs);
    needWait = !(ThreadState::state_is (*this, readyState)
      || ThreadState::state_is (*this, terminatedState));
    assert (!ThreadState::state_is (*this, destroyedState));
  }

  if (needWait) 
    wait ();

  { 
    SMutex::Lock lock (cs);
    ThreadState::move_to (*this, destroyedState);
  }

 LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "Destroy "; outString (oss_)
     );
  CloseHandle (handle);
}

void SThread::outString (std::ostream& out) const
{
  out << "SThread(id = "  
      << id ()
      << ", this = " 
      << std::hex << (void *) this << std::dec
      << ", currentState = " 
      << currentState.name ()
      << ')';
}


void SThread::wait()
{
  bool cntIncremented = false;
  try
  {
    { SMutex::Lock lock(cs);

    if (ThreadState::state_is (*this, terminatedState))
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
       SThread::current ().outString (oss_)
       );

    isTerminatedEvent.wait ();
    waitCnt--;
  }
  catch (...)
  {
     if (cntIncremented) waitCnt--;

     LOG4CXX_ERROR 
       (Logging::Root (), 
        "Unknown exception in SThread::wait");
  }
}

void SThread::start()
{
  SMutex::Lock lock(cs);
  ThreadState::check_moving_to 
    (*this, workingState);

  handle = (HANDLE) _beginthreadex
    ( 
      NULL,              // no security attribute 
      0,                 // default stack size 
      _helper,           // ThreadProc
      this,              // thread parameter 
      0,                 // not suspended 
      0 /*&dwThreadId*/ // returns thread ID     
     );

  // FIXME _current.set (0) on thread exit??

  sWinCheck(handle != 0, L"creating thread");

  ThreadState::move_to (*this, workingState);
}

void SThread::stop ()
{
  SMutex::Lock lock(cs);
  stopEvent.set ();
  exitRequested = true;
}


void SThread::_run()
{
   //TODO check run from current thread

   LOG4STRM_DEBUG
     (Logging::Thread (),
     outString (oss_); oss_ << " started");

  try
  {
    checkHR(CoInitialize(0));

    run();
  }
  catch ( XShuttingDown & )
  {
    // Do nothing - execution is finished
  }
  catch ( exception & x )
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
    SMutex::Lock lock(cs);
    
    ThreadState::move_to (*this, terminatedState);
  }

   LOG4STRM_DEBUG
     (Logging::Thread (), 
      outString (oss_); oss_ << " finished"
      );

  isTerminatedEvent.set ();
  if (externalTerminated) 
    externalTerminated->set ();
}

SThread & SThread::current()
{
  SThread * thread = reinterpret_cast<SThread*> 
    (SThreadBase::get_current ());

  assert (thread); //FIXME check
  return *thread;
}


// SThread::Tls  =====================================================

SThread::Tls::Tls() :
  idx(TlsAlloc())
{
  sWinCheck(idx != TLS_OUT_OF_INDEXES, L"creating TLS");
}

SThread::Tls::~Tls()
{
  TlsFree(idx);
}

void * SThread::Tls::get()
{
  void * data = TlsGetValue(idx);
  if ( data == 0 )
  {
    DWORD code = GetLastError();
    if ( code != NO_ERROR ) sWinErrorCode(code, L"getting TLS");
  }
  return data;
}
 
void SThread::Tls::set( void * data )
{
  if (data)
    LOG4STRM_DEBUG
      (Logging::Thread (),
         oss_ << "Unassociate ";
         SThread* thrd = (SThread*) get ();
         if (thrd) thrd->outString (oss_);
         else oss_ << "SThread(<null>)";
         oss_ << " with the system thread id = "
              << ::GetCurrentThreadId ();
       );

  sWinCheck(TlsSetValue(idx, data) != 0, L"setting TLS");

  if (data)
    LOG4STRM_DEBUG
      (Logging::Thread (),
       oss_ << "Associate ";
       if (data) ((SThread*) data) -> outString (oss_);
       else oss_ << "<null>";
       oss_ << " with the system thread id = "
            << ::GetCurrentThreadId ();
       );
}


//====================================================================

/*void sSleep( int time )
{
  static SEvent evt(true);
  evt.wait(time);
}*/
