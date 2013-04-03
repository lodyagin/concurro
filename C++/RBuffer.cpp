// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RBuffer.h"

DEFINE_STATES(RBuffer, DataBufferStateAxis, State)
({
  {   "charging", // is locked for filling
      "charged", // has data
		"discharged", // data was red (moved)
		"destroyed" // for disable destroying buffers with
						// data 
		},
  { {"discharged", "charging"},
	 {"charging", "charged"},
	 {"charging", "discharged"},
    {"charged", "discharged"},
	 {"discharged", "destroyed"}}
});

DEFINE_STATE_CONST(RBuffer, State, charged);
DEFINE_STATE_CONST(RBuffer, State, charging);
DEFINE_STATE_CONST(RBuffer, State, discharged);
DEFINE_STATE_CONST(RBuffer, State, destroyed);

REvent<DataBufferStateAxis> RBuffer
//
::is_discharged("discharged");

RBuffer::~RBuffer()
{
  // must be set to destroyed state in ancestor
  // destructors 
  State::ensure_state(*this, destroyedState);
}

RSingleBuffer::RSingleBuffer(size_t res)
  : buf(0), size_(0), reserved_(res) 
{}

RSingleBuffer::RSingleBuffer(RSingleBuffer&& b)
{
  reserved_ = b.reserved_;
  buf = b.buf;
  size_ = b.size_;
  State::move_to(b, dischargedState);
  b.buf = 0;
  b.size_ = 0;
  State::move_to(*this, chargedState);
}

RSingleBuffer::~RSingleBuffer()
{
#if 0
  try {
	 State::move_to(*this, destroyedState);
  }
  catch(const InvalidStateTransition& trans) 
  {
	 // switch on trans.from
  }
#else
  is_discharged.wait(*this);
  State::move_to(*this, destroyedState);
#endif

  delete[] buf;
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


