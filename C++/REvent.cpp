#include "StdAfx.h"
#include "REvent.h"
#include "Logging.h"
#include "RThread.h"
#ifdef _WIN32
#include "SShutdown.h"
#else
using namespace neosmart;
#endif
// REvtBase  =========================================================

REvtBase::REvtBase( HANDLE _h ) :
  h(_h)
{
#ifdef _WIN32
  LOG4STRM_DEBUG 
    (Logging::Thread (),
    oss_ << "Thread " << SThread::current ().id()
     << " creates the event handle " << h
     );
  sWinCheck(h != 0, L"creating an event");
#endif
}

REvtBase::~REvtBase()
{
#ifdef _WIN32
  LOG4STRM_DEBUG 
    (Logging::Thread (),
    oss_ << "Thread " << SThread::current ().id()
     << " closes the event handle " << h
     );
  if (!h) CloseHandle(h);
#else
  if (h) DestroyEvent(h);
#endif
  h = 0; 
}

void REvtBase::wait()
{
#ifdef _WIN32
  LOG4STRM_DEBUG 
    (Logging::Thread (),
    oss_ << "Thread " << SThread::current ().id()
     << " waits on handle " << h
     );
#endif
  HANDLE evts[] = {
#ifndef SHUTDOWN_UNIMPL
    SShutdown::instance().event(),
#endif
  h };
#ifdef _WIN32
  DWORD code = WaitForMultipleObjects(2, evts, false, INFINITE);
  if ( code == WAIT_OBJECT_0 ) xShuttingDown(L"REvent.wait");
  if ( code != WAIT_OBJECT_0 + 1 )
    sWinErrorCode(code, L"waiting for an event");
#else
  int code = WaitForMultipleEvents(evts, 1, false, (uint64_t) -1);
#endif
}

bool REvtBase::wait( int time )
{
#ifdef _WIN32
  LOG4STRM_DEBUG 
    (Logging::Thread (),
    oss_ << "Thread " << SThread::current ().id()
     << " waits of handle " << h
     << " for " << time << "msecs"
     );
#endif
  HANDLE evts[] = {
#ifndef SHUTDOWN_UNIMPL
  SShutdown::instance().event(),
#endif
  h };
#ifdef _WIN32
  DWORD code = WaitForMultipleObjects(2, evts, false, time);
  
  if ( code == WAIT_OBJECT_0 ) xShuttingDown(L"REvent.wait");
  if ( code == WAIT_TIMEOUT ) return false;
  if ( code != WAIT_OBJECT_0 + 1 ) 
    sWinErrorCode(code, L"waiting for an event");
#else
  int code = WaitForMultipleEvents(evts, 1, false, time);
#endif
  return true;
}


// REvent  ===========================================================
REvent::REvent( bool manual, bool init ) :
#ifdef _WIN32
  Parent(CreateEvent(0, manual, init, 0))
#else
  REvtBase(CreateEvent(manual, init))
#endif
{
}

void REvent::set()
{
#ifdef _WIN32
  LOG4STRM_DEBUG 
    (Logging::Thread (),
    oss_ << "Thread " << SThread::current ().id()
     << " set event on handle " << h
     );
  sWinCheck
    (SetEvent(h) != 0, 
     SFORMAT (L"setting event, handle = " << h).c_str ()
     );
#else
  SetEvent(h);
#endif
}

void REvent::reset()
{
#ifdef _WIN32
  LOG4STRM_DEBUG 
    (Logging::Thread (),
    oss_ << "Thread " << SThread::current ().id()
     << " reset event on handle " << h
     );
  sWinCheck
    (ResetEvent(h) != 0, 
     SFORMAT (L"resetting event, handle = " << h).c_str ()
     );
#else
  ResetEvent(h);
#endif
}


// SSemaphore  =======================================================
/*
SSemaphore::SSemaphore( int maxCount, int initCount ) :
  Parent(CreateSemaphore(0, initCount, maxCount, 0))
{
}

void SSemaphore::release( int count )
{
  sWinCheck(ReleaseSemaphore(h, count, 0) != 0, L"releasing semaphore");
}
*/

//====================================================================

size_t waitMultiple( HANDLE * evts, size_t count )
{
#ifdef _WIN32
  SPRECONDITION(count <= MAXIMUM_WAIT_OBJECTS);
  
  DWORD code = WaitForMultipleObjects(count, evts, false, INFINITE);
  
  if ( code < WAIT_OBJECT_0 || code >= WAIT_OBJECT_0 + count )
    sWinErrorCode(code, L"waiting for multiple objects");
  
  return code - WAIT_OBJECT_0;
#else
  int code = WaitForMultipleEvents(evts, count, false, 0);
  return code;// - WAIT_OBJECT_0;
#endif
}

size_t waitMultipleSD( HANDLE * _evts, size_t count )
{
#ifdef _WIN32
  SPRECONDITION(count <= MAXIMUM_WAIT_OBJECTS - 1);
  
  HANDLE evts[MAXIMUM_WAIT_OBJECTS];
#endif
#ifndef SHUTDOWN_UNIMPL
  evts[0] = SShutdown::instance().event();
  memcpy(evts + 1, _evts, count * sizeof(HANDLE));
  size_t idx = waitMultiple(evts, count + 1);
#else
  size_t idx = waitMultiple(_evts, count );
#endif

  if ( idx == 0 ) xShuttingDown("waitMultipleSD");
  return idx - 1;
}

