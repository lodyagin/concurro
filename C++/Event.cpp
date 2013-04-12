#include "StdAfx.h"
#include "REvent.hpp"
#include "Logging.h"
#include "RThread.h"
#ifdef _WIN32
#include "SShutdown.h"
#else
using namespace neosmart;
#endif

// EvtBase  ===============================================

EvtBase::EvtBase(const std::string& id, HANDLE h_ ) :
  universal_object_id(id), h(h_)
{
  LOG_DEBUG(log, "Thread " << "<todo>"
				<< " creates the event ["
				<< universal_object_id << "]");
#ifdef _WIN32
  sWinCheck(h != 0, L"creating an event");
#endif
}

EvtBase::~EvtBase()
{
  LOG_DEBUG(log, "Thread " << "<todo>"
				<< " closes the event handle [" 
				<< universal_object_id << "]");

#ifdef _WIN32
  if (!h) CloseHandle(h);
#else
  if (h) DestroyEvent(h);
#endif
  h = 0; 
}

void EvtBase::wait() const
{
#ifdef _WIN32
  wait(INFINITE);
#else
  //wait(std::numeric_limits<uint64_t>::max());
  wait(-1);
#endif
}

bool EvtBase::wait(int time) const
{
//  if (time != std::numeric_limits<uint64_t>::max()) {
  if (time != -1) {
	 LOG_TRACE(log, "Thread " << "<todo>"
				  << " waits for the event " << *this
				  << " for " << time << " msecs"
		);
  }
  else {
	 LOG_TRACE(log, "Thread " << "<todo>"
				  << " waits for the event " << *this
				  << " w/o timeout"
		);
  }

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
  int code = WaitForEvent(evts[0], time);
  if (code == ETIMEDOUT) return false;
#endif
  return true;
}

std::ostream&
operator<< (std::ostream& out, const EvtBase& evt)
{
  out << '[' << evt.universal_object_id << ']';
  return out;
}

// Event  ===========================================================
Event::Event(const std::string& id, bool manual, bool init )
  :
#ifdef _WIN32
  Parent(id, CreateEvent(0, manual, init, 0))
#else
  Parent(id, CreateEvent(manual, init)),
  is_signalled(false), is_manual(manual)
#endif
{
}

void Event::set()
{
  LOG_TRACE(log, "Thread " << "<todo>"
				<< " is setting the " << *this << " event");
#ifdef _WIN32
  sWinCheck
    (SetEvent(h) != 0, 
     SFORMAT (L"setting event, handle = " << h).c_str ()
     );
#else
  SetEvent(h);
  is_signalled = true;
#endif
}

void Event::reset()
{
  LOG_TRACE(log, "Thread " << "<todo>"
				<< " is resetting the " << *this << " event");
#ifdef _WIN32
  sWinCheck
    (ResetEvent(h) != 0, 
     SFORMAT (L"resetting event, handle = " << h).c_str ()
     );
#else
  is_signalled = false;
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

