// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RBuffer.h"
#include "RWindow.h"
#include "REvent.hpp"
#include "RState.hpp"

DEFINE_AXIS(
  DataBufferStateAxis,
  {   "dummy", // No memory reserved, will copy limits
               // from a source buffer.
      "charging", // is locked for filling
      "charged", // has data
      "discharged", // data was red (moved)
      "moving_destination",
      "moving_source",
      "resizing_discharged",
      "bottom_extending",
      "bottom_extended"
      },
  { {"dummy", "moving_destination"},
    {"moving_destination", "charged"},
    {"discharged", "charging"},
    {"discharged", "resizing_discharged"},
    {"discharged", "moving_destination"},
    {"resizing_discharged", "discharged"},
    {"charging", "charged"},
    {"charged", "discharged"},
    {"charged", "bottom_extending"},
    {"charged", "moving_source"},
    {"bottom_extending", "bottom_extended"},
    {"moving_source", "discharged"},
      //{"charging", "discharged"}, 
      // there is a week control if its enabled
    {"bottom_extended", "discharged"} //<NB> no bottom_extended->charged
  });

DEFINE_STATES(DataBufferStateAxis);

DEFINE_STATE_CONST(RBuffer, State, dummy);
DEFINE_STATE_CONST(RBuffer, State, charging);
DEFINE_STATE_CONST(RBuffer, State, charged);
DEFINE_STATE_CONST(RBuffer, State, discharged);
DEFINE_STATE_CONST(RBuffer, State, moving_destination);
DEFINE_STATE_CONST(RBuffer, State, moving_source);
DEFINE_STATE_CONST(RBuffer, State, resizing_discharged);
DEFINE_STATE_CONST(RBuffer, State, bottom_extending);
DEFINE_STATE_CONST(RBuffer, State, bottom_extended);

RBuffer::RBuffer
(const RState<DataBufferStateAxis>& initial_state) 
: Parent(initial_state),
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
  : RBuffer(dummyState),
    buf(0), size_(0), 
    top_reserved_(0), bottom_reserved_(0)
{
}

RSingleBuffer::RSingleBuffer(size_t res, size_t bottom_res)
  : RBuffer(dischargedState),
    buf(0), size_(0), 
    top_reserved_(res), bottom_reserved_(bottom_res) 
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

  State::move_to(*this, moving_destinationState);
  State::move_to(*from, moving_sourceState);
  top_reserved_ = b->top_reserved_;
  bottom_reserved_ = b->bottom_reserved_;
  buf = b->buf; size_ = b->size_;
  State::move_to(*this, chargedState);
  State::move_to(*from, dischargedState);
}

void RSingleBuffer::reserve(size_t res)
{
  SCHECK(res > 0 && res >= size());
  STATE(RSingleBuffer, move_to, resizing_discharged);
  top_reserved_ = res;
  STATE(RSingleBuffer, move_to, discharged);
}

void* RSingleBuffer::data() 
{ 
  start_charging();
  assert(buf);
  return buf + bottom_reserved_; 
}

void RSingleBuffer::resize(size_t sz) 
{ 
  if (sz > top_reserved_)
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
  if (!buf) {
    const size_t res = top_reserved_+ bottom_reserved_;
    try {
      buf = new char[res];
    }
    catch (const std::bad_alloc& ex) {
      LOG_ERROR(log, "Unable to allocate RSingleBuffer "
                << "of size " << res);
      throw ex;
    }
  }
  State::move_to(*this, chargingState);
}

void RSingleBuffer::extend_bottom(const RWindow& wnd)
{
  SCHECK(wnd.size() <= bottom_reserved_);
  STATE(RSingleBuffer, move_to, bottom_extending);
  memcpy(buf + bottom_reserved_ - wnd.size(), &wnd[0],
         wnd.size());
  STATE(RSingleBuffer, move_to, bottom_extended);
}
