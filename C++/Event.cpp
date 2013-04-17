// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "REvent.h"
#include "Logging.h"
#include "RThread.h"
#ifdef _WIN32
#include "SShutdown.h"
#else
using namespace neosmart;
#endif

// EvtBase  ===============================================

EvtBase::EvtBase(const std::string& id, 
					  bool manual, 
					  bool init )
  : universal_object_id(id),
	 /*is_signalled(init),*/ 
	 shadow(false),
	 is_manual(manual),
#ifdef _WIN32
  h(CreateEvent(0, manual, init, 0))
#else
  h(CreateEvent(manual, init))
#endif
{
  LOG_DEBUG(log, "thread " 
				<< std::this_thread::get_id()
				<< "> event "
				<< universal_object_id << "> created");
#ifdef _WIN32
  sWinCheck(h != 0, L"creating an event");
#endif
}

EvtBase::~EvtBase()
{
  LOG_DEBUG(log, "thread " 
				<< std::this_thread::get_id()
				<< "> closes the event handle [" 
				<< universal_object_id << "]");

#ifdef _WIN32
  if (!h) CloseHandle(h);
#else
  if (h) DestroyEvent(h);
#endif
  h = 0; 
}

std::ostream&
operator<< (std::ostream& out, const EvtBase& evt)
{
  out << '[' << evt.universal_object_id << ']';
  return out;
}

void EvtBase::set()
{
  LOG_DEBUG(log, "thread " 
				<< std::this_thread::get_id()
				<< "> event "
				<< universal_object_id << "> set");
#ifdef _WIN32
  sWinCheck
    (SetEvent(h) != 0, 
     SFORMAT (L"setting event, handle = " << h).c_str ()
     );
#else
  //is_signalled = true;
  shadow = true; //<NB> before SetEvent to prevent a "no
					  //shadow and no event" case
  SetEvent(h);
#endif
}

void EvtBase::reset()
{
  LOG_DEBUG(log, "thread " 
				<< std::this_thread::get_id()
				<< "> event "
				<< universal_object_id << "> reset");
#ifdef _WIN32
  sWinCheck
    (ResetEvent(h) != 0, 
     SFORMAT (L"resetting event, handle = " << h).c_str ()
     );
#else
  //is_signalled = false;
  ResetEvent(h);
#endif
}

bool EvtBase::wait_impl(int time) const
{
//  if (time != std::numeric_limits<uint64_t>::max()) {
  if (time != -1) {
	 LOG_DEBUG(log, "thread " 
				  << std::this_thread::get_id()
				  << "> event "
				  << universal_object_id 
				  << "> wait " << time << " msecs");
  }
  else {
	 LOG_DEBUG(log, "thread " 
				  << std::this_thread::get_id()
				  << "> event "
				  << universal_object_id 
				  << "> waits w/o timeout");
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
  if (code == ETIMEDOUT) {
	 LOG_DEBUG(log, "thread " 
				  << std::this_thread::get_id()
				  << "> event "
				  << universal_object_id 
				  << "> wait: timed out");
	 return false;
  }
#endif
  LOG_DEBUG(log, "thread " 
				<< std::this_thread::get_id()
				<< "> event "
				<< universal_object_id 
				<< "> wait: signalled");
  return true;
}

CompoundEvent::CompoundEvent()
  : vector_need_update(false), //<NB>
	 has_autoreset(false)
{}

CompoundEvent::CompoundEvent(CompoundEvent&& e)
  : handle_set(std::move(e.handle_set)),
	 handle_vec(std::move(e.handle_vec)),
	 vector_need_update(e.vector_need_update),
	 has_autoreset(e.has_autoreset)
{}

CompoundEvent::CompoundEvent(const CompoundEvent& e)
  : handle_set(e.handle_set),
	 handle_vec(e.handle_vec),
	 vector_need_update(e.vector_need_update),
	 has_autoreset(e.has_autoreset)
{}

CompoundEvent::CompoundEvent(const Event& e)
  : handle_set{e}, vector_need_update(true),
    has_autoreset(!e.is_manual())
{}

CompoundEvent::CompoundEvent
 (std::initializer_list<Event> evs)
	: vector_need_update(evs.size() > 0),
	  has_autoreset(false)
{
  for (const Event& e : evs) {
	 handle_set.insert(e);
	 has_autoreset = has_autoreset || !e.is_manual();
  }
}


CompoundEvent& CompoundEvent
::operator= (CompoundEvent&& e)
{
  handle_set = std::move(e.handle_set);
  handle_vec = std::move(e.handle_vec);
  vector_need_update = e.vector_need_update;
  has_autoreset = e.has_autoreset;
  return *this;
}

const CompoundEvent& CompoundEvent
::operator|= (const Event& e)
{
  handle_set.insert(e);
  vector_need_update = true;
  has_autoreset = has_autoreset || !e.is_manual();
  return *this;
}

#if 0
CompoundEvent& CompoundEvent
::operator|= (const CompoundEvent& e)
{
  handle_set.insert(e.handle_set.begin(), 
						  e.handle_set.end());
  vector_need_update = true;
  has_autoreset = has_autoreset || e.has_autoreset;
  return *this;
}
#endif

bool CompoundEvent::wait_impl(int time) const
{
  if (time != -1) {
	 LOG_DEBUG(log, "thread " 
				  << std::this_thread::get_id()
				  << "> event " << *this
				  << "> wait " << time << " msecs");
  }
  else {
	 LOG_DEBUG(log, "thread " 
				  << std::this_thread::get_id()
				  << "> event " << *this
				  << "> waits w/o timeout");
  }

  update_vector();
  assert(handle_vec.size());

  const int code = WaitForMultipleEvents(&handle_vec[0], 
											handle_vec.size(),
											false, // wait for any
											time);

  if (code == ETIMEDOUT) {
	 LOG_DEBUG(log, "thread " 
				  << std::this_thread::get_id()
				  << "> event " << *this
				  << "> wait: timed out");
	 return false;
  }
  LOG_DEBUG(log, "thread " 
				<< std::this_thread::get_id()
				<< "> event " << *this
				<< "> wait: signalled");
  return true;
}

void CompoundEvent::update_vector() const
{
  if (!vector_need_update)
	 return;

  handle_vec.reserve(handle_set.size());
  std::transform
	 (handle_set.begin(), handle_set.end(),
	  std::back_inserter(handle_vec),
	  [](const Event& e) { return e.evt_ptr->h; });

  vector_need_update = false;
}

std::ostream&
operator<< (std::ostream& out, const CompoundEvent& ce)
{
  out << "{";
  bool first = true;
  for (auto& ev : ce.handle_set) {
	 if (!first) out << " | "; else first = false;
	 out << ev.universal_id();
  }
  out << "}";
  return out;
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

#if 0
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
#endif
