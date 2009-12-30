#include "stdafx.h"
#include "SThread.h"
#include "SEvent.h"
#include "SShutdown.h"
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
  {"ready", "destroyed"}
  // can't be destroyed in other states

};

Logging SThread::m_Logging("SThread");

// SThread  ==========================================================

SThread::SThread() :
  handle(0),
  _id(++counter),
  stateMap (0)
{
  initializeStates ();
}

SThread::SThread( Main ) :
  handle(0),
  _id(0),
  stateMap (0)
{
  _current.set(this);
  initializeStates ();
}

SThread::SThread( External ) :
  handle(0),
  _id(-1),
  stateMap (0)
{
  _current.set(this);
  initializeStates ();
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

  currentState = readyState;
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
}

SThread::~SThread()
{
  move_to (destroyedState);
  if ( _id <= 0 ) _current.set(0);
  CloseHandle (handle);
}

void SThread::wait()
{
  if (!stateMap->there_is_transition 
      (currentState, waitingState)
      )
    return; // Shouldn't wait, it is ready

  try
  {
     move_to (waitingState);
      //LOG4CXX_DEBUG(AutoLogger.GetLogger(),sFormat("Waiting for thread [%02u]", id()));
     sWinCheck(
        WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0, 
        "waiting on thread termination");
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
  check_moving_to (workingState);

  handle = CreateThread( 
            NULL,              // no security attribute 
            0,                 // default stack size 
            _helper,           // ThreadProc
            this,              // thread parameter 
            0,                 // not suspended 
            0 /*&dwThreadId*/);      // returns thread ID     
  sWinCheck(handle != 0, "creating thread");

  move_to (workingState);
}

void SThread::stop ()
{
  move_to (exitRState);
}


void SThread::_run()
{
    static Logging AutoLogger("_run()",m_Logging);

  _current.set(this);

   LOG4CXX_DEBUG(AutoLogger.GetLogger(),"Thread started");

  try
  {
    checkHR(CoInitialize(0));

    run();
  }
  catch ( XShuttingDown & )
  {
    //FIXME
  }
  catch ( exception & x )
  {
      LOG4CXX_ERROR(AutoLogger.GetLogger(),sFormat("Internal server error: thread failed: %s", x.what()));
     SShutdown::instance().shutdown();
  }
  catch ( ... )
  {
      LOG4CXX_ERROR(AutoLogger.GetLogger(),"Internal server error: thread failed");
     SShutdown::instance().shutdown();
  }
   LOG4CXX_DEBUG(AutoLogger.GetLogger(),"Thread finished");
  _current.set(0);

  if (currentState == workingState)
    move_to (selfTerminatedState);
  else
    move_to (terminatedByRState);
}

DWORD WINAPI SThread::_helper( void * p )
{
  SThread * _this = reinterpret_cast<SThread *>(p);
  _this->_run();
  return 0;
}

SThread & SThread::current()
{
  SThread * thread = static_cast<SThread *>(_current.get());
  assert (thread);
  return *thread;
}

bool SThread::is_stop_requested ()
{
  return currentState == exitRState;
}


SMTCounter SThread::counter(0);
SThread::Tls SThread::_current;


// SThread::Tls  =====================================================

SThread::Tls::Tls() :
  idx(TlsAlloc())
{
  sWinCheck(idx != TLS_OUT_OF_INDEXES, "creating TLS");
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
    if ( code != NO_ERROR ) sWinErrorCode(code, "getting TLS");
  }
  return data;
}
 
void SThread::Tls::set( void * data )
{
  sWinCheck(TlsSetValue(idx, data) != 0, "setting TLS");
}


//====================================================================

void sSleep( int time )
{
  static SEvent evt(true);
  evt.wait(time);
}
