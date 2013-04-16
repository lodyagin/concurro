// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RBuffer.h"

DEFINE_STATES(DataBufferStateAxis, 
  {   "charging", // is locked for filling
      "charged", // has data
		"discharged", // data was red (moved)
		"destroyed",// for disable destroying buffers with
						// data 
		"welded"  // own a buffer together with another
					 // RBuffer
		},
  { {"discharged", "charging"},
	 {"charging", "charged"},
	 {"discharged", "welded"},
	 {"welded", "discharged"},
	 {"welded", "charged"},
    {"charged", "discharged"},
	 {"discharged", "destroyed"}}
);

DEFINE_STATE_CONST(RBuffer, State, charged);
DEFINE_STATE_CONST(RBuffer, State, charging);
DEFINE_STATE_CONST(RBuffer, State, discharged);
DEFINE_STATE_CONST(RBuffer, State, destroyed);
DEFINE_STATE_CONST(RBuffer, State, welded);

RBuffer::~RBuffer()
{
  // must be set to destroyed state in ancestor
  // destructors 
  State::ensure_state(*this, destroyedState);
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
  is_discharged_event.wait();
  State::move_to(*this, destroyedState);
  delete[] buf;
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
	 buf = new char[reserved_];
  State::move_to(*this, chargingState);
}
