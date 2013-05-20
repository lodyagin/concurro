// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RBuffer.h"
#include "REvent.hpp"
#include "RState.hpp"

DEFINE_AXIS(
  DataBufferStateAxis,
  {   "charging", // is locked for filling
      "charged", // has data
      "discharged", // data was red (moved)
      "welded"  // own a buffer together with another
                // RBuffer
      },
  { {"discharged", "charging"},
    {"charging", "charged"},
      //{"charging", "discharged"}, 
      // there is a week control if its enabled
    {"discharged", "welded"},
    {"welded", "discharged"},
    {"welded", "charged"},
    {"charged", "discharged"}}
  );

DEFINE_STATES(DataBufferStateAxis);

DEFINE_STATE_CONST(RBuffer, State, charged);
DEFINE_STATE_CONST(RBuffer, State, charging);
DEFINE_STATE_CONST(RBuffer, State, discharged);
DEFINE_STATE_CONST(RBuffer, State, welded);

RBuffer::RBuffer() 
: RObjectWithEvents<DataBufferStateAxis>
	 (dischargedState),
  CONSTRUCT_EVENT(charged),
  CONSTRUCT_EVENT(discharged),
  destructor_is_called(false),
  autoclear(false)
{}

RBuffer::~RBuffer()
{
  SCHECK(destructor_is_called);
}

RSingleBuffer::RSingleBuffer()
  : buf(0), size_(0), reserved_(0) 
{}

RSingleBuffer::RSingleBuffer(size_t res)
  : buf(0), size_(0), reserved_(res) 
{}

RSingleBuffer::RSingleBuffer(RSingleBuffer&& b)
{
  *this = std::move(b);
}

RSingleBuffer::~RSingleBuffer()
{
  if (autoclear)
	 clear();
  else
	 is_discharged_event.wait();
  delete[] buf;
  destructor_is_called = true;
}

RSingleBuffer& RSingleBuffer
//
::operator= (RSingleBuffer&& b)
{
  State::move_to(*this, weldedState);
  const size_t old_reserved = reserved_;
  reserved_ = b.reserved_;
  buf = b.buf; size_ = b.size_;
  try {
	 State::move_to(b, dischargedState);
  }
  catch (const InvalidStateTransition&)
  {
	 reserved_ = old_reserved;
	 buf = 0; size_ = 0;
	 State::move_to(*this, dischargedState);
	 throw;
  }
  b.buf = 0; b.size_ = 0;
  State::move_to(*this, chargedState);
  return *this;
}

void RSingleBuffer::reserve(size_t res)
{
  SCHECK(res > 0);
  // Unable to resize an existing buffer.
  State::ensure_state(*this, dischargedState);
  reserved_ = res;
}

void* RSingleBuffer::data() 
{ 
  start_charging();
  assert(buf);
  return buf; 
}

void RSingleBuffer::resize(size_t sz) 
{ 
  if (sz > reserved_)
	 throw ResizeOverCapacity();

  if (sz > 0) {
	 State::ensure_state(*this, chargingState);
	 SCHECK(buf);
	 size_ = sz;
	 State::move_to(*this, chargedState);
  }
  else {
	 State::move_to(*this, dischargedState);
	 delete[] buf;
	 buf = 0;
	 size_ = sz;
  }
}

void RSingleBuffer::start_charging()
{
  size_ = 0;
  if (!buf)
	 try {
		buf = new char[reserved_];
	 }
	 catch (const std::bad_alloc& ex) {
		LOG_ERROR(log, "Unable to allocate RSingleBuffer "
					 << "of size " << reserved_);
		throw ex;
	 }
  State::move_to(*this, chargingState);
}
