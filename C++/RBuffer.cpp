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
      "welded",  // own a buffer together with another
                 // RBuffer
      "resizing_charged",
      "resizing_discharged"
      },
  { {"discharged", "charging"},
    {"charging", "charged"},
      //{"charging", "discharged"}, 
      // there is a week control if its enabled
    {"discharged", "welded"},
    {"welded", "discharged"},
    {"welded", "charged"},
    {"charged", "discharged"},
    {"charged", "resizing_charged"},
    {"resizing_charged", "charged"},
    {"discharged", "resizing_discharged"},
    {"resizing_discharged", "discharged"},
  });

DEFINE_STATES(DataBufferStateAxis);

DEFINE_STATE_CONST(RBuffer, State, charged);
DEFINE_STATE_CONST(RBuffer, State, charging);
DEFINE_STATE_CONST(RBuffer, State, discharged);
DEFINE_STATE_CONST(RBuffer, State, welded);

RBuffer::RBuffer() 
: Parent(dischargedState),
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

RSingleBuffer::RSingleBuffer(RBuffer* buf)
  : RSingleBuffer()
{
  this->move(buf);
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

void RSingleBuffer::move(RBuffer* from)
{
  assert(from);
  RSingleBuffer* b = dynamic_cast<RSingleBuffer*>(from);
  if (!b) 
    THROW_NOT_IMPLEMENTED;

  State::move_to(*this, weldedState);
  const size_t old_reserved = reserved_;
  reserved_ = b->reserved_;
  buf = b->buf; size_ = b->size_;
  try {
    State::move_to(*b, dischargedState);
  }
  catch (const InvalidStateTransition&)
  {
    reserved_ = old_reserved;
    buf = 0; size_ = 0;
    State::move_to(*this, dischargedState);
    throw;
  }
  b->buf = 0; b->size_ = 0;
  State::move_to(*this, chargedState);
}

void RSingleBuffer::reserve(size_t res, size_t shift)
{
  SCHECK(res > 0);
  SCHECK(res >= size() + shift);
  do {
    if (State::compare_and_move(
          *this, dischargedState, resizing_dischargedState))
    {
      reserved_ = res;
      STATE(RSingleBuffer, move_to, discharged);
      return;
    }
    else if (State::compare_and_move(
               *this, chargedState, 
               resizing_chargedState))
    {
      SCHECK(res >= size());
      buf.reserve(res);
      if (move > 0) {
        std::move_backward(
          buf.begin(), 
          buf.begin() + size(),
          buf.begin() + size() + shift);
      }
      STATE(RSingleBuffer, move_to, charged);
      return;
    }
    (is_discharged() | is_charged()).wait();
  } while (true);
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

RSingleBuffer* join_buffers
  (RSingleBuffer* buf,
   size_t offset, 
   RSingleBuffer* buf2)
{
  assert(buf->size() >= offset);
  const size_t add = buf->size() - offset;
  if (add > 0) {
    RSingleBuffer* new_buf = new RSingleBuffer(buf2);
    new_buf->reserve(new_buf->size() + add);
}
