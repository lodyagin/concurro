#include "stdafx.h"
#include "SThread.h"
#include <objbase.h>


using namespace std;

const State2Idx SThread::allStates[] =
{
  {1, "ready"},         // after creation
  {2, "working"},       
  {3, "exitRequested"}, 
  {4, "waiting"},       
  {5, "selfTerminated"},    
  {6, "terminatedByRequest"},    
  {7, "destroyed"},    // to check a state in the destructor
  {0, 0}
};

const StateTransition SThread::allTrans[] =
{
  {"ready", "working"},      // start ()
  {"working", "exitRequested"}, // stop  ()

  {"working", "selfTerminated"}, // natural terminating

  {"exitRequested", "waiting"}, // wait ()
  
  {"exitRequested", "terminatedByRequest"}, 
  // when asked to terminate and it 
  // quickly completes its work
  {"waiting", "terminatedByRequest"},
  // the same, but another thread already waits
  // our termination

  {"selfTerminated", "destroyed"},
  {"terminatedByRequest", "destroyed"},
  {"ready", "destroyed"},
  // can't be destroyed in other states
  {0, 0}

};

// SThread  ==========================================================

StateMap* SThread::stateMap = 0;

UniversalState SThread::readyState;
UniversalState SThread::workingState;
UniversalState SThread::exitRState;
UniversalState SThread::waitingState;
UniversalState SThread::selfTerminatedState;
UniversalState SThread::terminatedByRState;
UniversalState SThread::destroyedState;

static int initStateMap = 
  (SThread::initializeStates(), 1);

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

SThread::SThread() :
  handle(0),
  isTerminatedEvent (false),
  _id(++counter),
  selfDestroing (true),
  stopEvent (true, false)
{

  currentState = readyState;
  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

SThread::SThread( Main ) :
  handle(0),
  isTerminatedEvent (false),
  _id(0),
  selfDestroing (false),
  stopEvent (true, false)
{
  currentState = readyState;
  _current.set(this);
  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

SThread::SThread( External ) :
  handle(0),
  isTerminatedEvent (false),
  _id(-1),
  selfDestroing (true),
  stopEvent (true, false)
{
  currentState = readyState;
  _current.set(this);
  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "New "; outString (oss_)
     );
}

void SThread::initializeStates ()
{
  stateMap = new StateMap(allStates, allTrans);
  readyState = stateMap->create_state ("ready");
  workingState = stateMap->create_state ("working");
  exitRState = stateMap->create_state ("exitRequested");
  waitingState = stateMap->create_state ("waiting");
  terminatedByRState = stateMap->create_state 
    ("terminatedByRequest");
  selfTerminatedState = stateMap->create_state 
    ("selfTerminated");
  destroyedState = stateMap->create_state ("destroyed");
}

inline void SThread::check_moving_to 
  (const UniversalState& to)
{
  stateMap->check_transition (currentState, to);
}

inline void SThread::move_to
  (const UniversalState& to)
{
  stateMap->check_transition (currentState, to);
  currentState = to;
  LOG4STRM_TRACE (Logging::Thread (), outString (oss_));
}

SThread::~SThread()
{
  static SMutex cs;

  { SMutex::Lock lock (cs);

    move_to (destroyedState);
  }

  if ( _id <= 0 ) _current.set(0);

  LOG4STRM_DEBUG
    (Logging::Thread (),
     oss_ << "Destroy "; outString (oss_)
     );
  CloseHandle (handle);
}

void SThread::outString (std::ostream& out) const
{
  out << "SThread(id = "  
      << _id
      << ", this = " 
      << std::hex << (void *) this << std::dec
      << ", currentState = " 
      << stateMap->get_state_name (currentState)
      << ')';
}


void SThread::wait()
{
  static SMutex cs;
  try
  {
    { SMutex::Lock lock(cs);

     if (!stateMap->there_is_transition 
        (currentState, waitingState)
        )
      return; // Shouldn't wait, it is ready

     move_to (waitingState);
    }

      //LOG4CXX_DEBUG(AutoLogger.GetLogger(),sFormat("Waiting for thread [%02u]", id()));
     isTerminatedEvent.wait ();
      //LOG4CXX_DEBUG(AutoLogger.GetLogger(),sFormat("Waiting for thread [%02u] - done", id()));
   }
  catch (...)
  {
     LOG4CXX_ERROR 
       (Logging::Root (), 
        "Unknown exception in SThread::wait");
  }
}

void SThread::start()
{
  static SMutex cs;
  SMutex::Lock lock(cs);
  check_moving_to (workingState);

  handle = (HANDLE) _beginthreadex
    ( 
      NULL,              // no security attribute 
      0,                 // default stack size 
      _helper,           // ThreadProc
      this,              // thread parameter 
      0,                 // not suspended 
      0 /*&dwThreadId*/ // returns thread ID     
     );

  sWinCheck(handle != 0, L"creating thread");

  move_to (workingState);
}

void SThread::stop ()
{
  static SMutex cs;
  SMutex::Lock lock(cs);
  stopEvent.set ();
  move_to (exitRState);
}


void SThread::_run()
{
   //static Logging AutoLogger("_run()",m_Logging);

  _current.set(this);

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
   LOG4STRM_DEBUG
     (Logging::Thread (), 
      outString (oss_); oss_ << " finished"
      );
  _current.set(0);

  if (currentState == workingState)
    move_to (selfTerminatedState);
  else
    move_to (terminatedByRState);

  isTerminatedEvent.set ();
}

unsigned int __stdcall SThread::_helper( void * p )
{
  SThread * _this = reinterpret_cast<SThread *>(p);
  _this->_run();
  delete _this;
  return 0;
}

SThread & SThread::current()
{
  SThread * thread = static_cast<SThread *>(_current.get());
  assert (thread); //FIXME check
  return *thread;
}

bool SThread::is_stop_requested ()
{
  static SMutex cs;
  SMutex::Lock lock(cs);
  return currentState == exitRState
    || currentState == waitingState;
}


SMTCounter SThread::counter(0);
SThread::Tls SThread::_current;


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

void sSleep( int time )
{
  static SEvent evt(true);
  evt.wait(time);
}
