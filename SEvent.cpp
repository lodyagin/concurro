#include "stdafx.h"
#include "SEvent.h"
#include "SShutdown.h"


// SEvtBase  =========================================================

SEvtBase::SEvtBase( HANDLE _h ) :
  h(_h)
{
  sWinCheck(h != 0, L"creating an event");
}

SEvtBase::~SEvtBase()
{
  CloseHandle(h);
}

void SEvtBase::wait()
{
  HANDLE evts[] = { SShutdown::instance().event(), h };

  DWORD code = WaitForMultipleObjects(2, evts, false, INFINITE);
  
  if ( code == WAIT_OBJECT_0 ) xShuttingDown(L"SEvent.wait");
  if ( code != WAIT_OBJECT_0 + 1 ) 
    sWinErrorCode(code, L"waiting for an event");
}

bool SEvtBase::wait( int time )
{
  HANDLE evts[] = { SShutdown::instance().event(), h };

  DWORD code = WaitForMultipleObjects(2, evts, false, time);
  
  if ( code == WAIT_OBJECT_0 ) xShuttingDown(L"SEvent.wait");
  if ( code == WAIT_TIMEOUT ) return false;
  if ( code != WAIT_OBJECT_0 + 1 ) 
    sWinErrorCode(code, L"waiting for an event");
  return true;
}


// SEvent  ===========================================================

SEvent::SEvent( bool manual, bool init ) :
  Parent(CreateEvent(0, manual, init, 0))
{
}

void SEvent::set()
{
  sWinCheck(SetEvent(h) != 0, L"setting event");
}

void SEvent::reset()
{
  sWinCheck(ResetEvent(h) != 0, L"resetting event");
}


// SSemaphore  =======================================================

SSemaphore::SSemaphore( int maxCount, int initCount ) :
  Parent(CreateSemaphore(0, initCount, maxCount, 0))
{
}

void SSemaphore::release( int count )
{
  sWinCheck(ReleaseSemaphore(h, count, 0) != 0, L"releasing semaphore");
}


//====================================================================

size_t waitMultiple( HANDLE * evts, size_t count )
{
  SPRECONDITION(count <= MAXIMUM_WAIT_OBJECTS);
  
  DWORD code = WaitForMultipleObjects(count, evts, false, INFINITE);
  
  if ( code < WAIT_OBJECT_0 || code >= WAIT_OBJECT_0 + count )
    sWinErrorCode(code, L"waiting for multiple objects");
  
  return code - WAIT_OBJECT_0;
}

size_t waitMultipleSD( HANDLE * _evts, size_t count )
{
  SPRECONDITION(count <= MAXIMUM_WAIT_OBJECTS - 1);
  
  HANDLE evts[MAXIMUM_WAIT_OBJECTS];
  evts[0] = SShutdown::instance().event();
  memcpy(evts + 1, _evts, count * sizeof(HANDLE));

  size_t idx = waitMultiple(evts, count + 1);

  if ( idx == 0 ) xShuttingDown(L"waitMultipleSD");
  return idx - 1;
}
