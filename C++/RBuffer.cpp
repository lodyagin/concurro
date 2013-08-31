/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RBuffer.h"
#include "RWindow.h"
#include "REvent.hpp"
#include "RState.hpp"

namespace curr {

DEFINE_AXIS(
  DataBufferStateAxis,
  {   "dummy", // No memory reserved, will copy limits
               // from a source buffer.
      "charging", // is locked for filling
      "charged", // has data
      "discharged", // data was red (moved)
      "undoing", // no data was red after start charging
      "moving_destination",
      "moving_source",
      "resizing_discharged",
      "bottom_extending",
      "bottom_extended"
      },
  { {"dummy", "moving_destination"},
    {"dummy", "resizing_discharged"}, // reserve
    {"moving_destination", "dummy"},     // another side
                                         // is not ready
    {"moving_destination", "charged"},
    {"discharged", "charging"},
    {"discharged", "resizing_discharged"},
    {"discharged", "moving_destination"},
    {"moving_destination", "discharged"}, // another side
                                          // is not ready
    {"resizing_discharged", "discharged"},
    {"charging", "charged"},
    {"charged", "discharged"},
    {"charged", "bottom_extending"},
    {"charged", "moving_source"},
    {"moving_source", "charged"},
    {"charged", "moving_source"},         // another side
                                          // is not ready
    {"bottom_extending", "bottom_extended"},
    {"moving_source", "discharged"},      
      //{"charging", "discharged"}, 
      // there is a week control if its enabled
      // Below is an alternative.
    {"charging", "undoing"},  // by cancel_charging
    {"undoing", "discharged"},// by cancel_charging

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
DEFINE_STATE_CONST(RBuffer, State, undoing);

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
    CONSTRUCT_EVENT(dummy),
    buf(0), size_(0), 
    top_reserved_(0), bottom_reserved_(0)
{
}

RSingleBuffer::RSingleBuffer(size_t res, size_t bottom_res)
  : RBuffer(dischargedState),
    CONSTRUCT_EVENT(dummy),
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
    (is_discharged_event | is_dummy_event).wait();
  delete[] buf;
  destructor_is_called = true;
}

void RSingleBuffer::move(RBuffer* from)
{
  assert(from);
  RSingleBuffer* b = dynamic_cast<RSingleBuffer*>(from);
  if (!b) 
    THROW_NOT_IMPLEMENTED;

  RState<DataBufferStateAxis> dest_0state(*this);
  RState<DataBufferStateAxis> src_0state(*from);
  try {
    State::move_to(*this, moving_destinationState);
    State::move_to(*from, moving_sourceState);
    top_reserved_ = b->top_reserved_;
    bottom_reserved_ = b->bottom_reserved_;
    buf = b->buf; size_ = b->size_;
    b->buf = nullptr; b->size_ = 0;
  }
  catch (const InvalidStateTransition&)
  {
    State::compare_and_move(
      *this, moving_destinationState, dest_0state);
    State::compare_and_move(
      *from, moving_sourceState, src_0state);
    throw;
  }
  State::move_to(*this, chargedState);
  State::move_to(*from, dischargedState);
}

void RSingleBuffer::reserve(size_t top, size_t bottom)
{
  SCHECK(top > 0 && top >= size());
  STATE(RSingleBuffer, move_to, resizing_discharged);
  top_reserved_ = top;
  bottom_reserved_ = bottom;
  STATE(RSingleBuffer, move_to, discharged);
}

void* RSingleBuffer::data() 
{ 
  start_charging();
  assert(buf);
  return buf + bottom_reserved_; 
}

const void* RSingleBuffer::cdata() const
{ 
  return (buf) ? buf + bottom_reserved_ : nullptr;
}

void RSingleBuffer::resize(size_t sz) 
{ 
  if (sz > top_reserved_)
    throw ResizeOverCapacity();

  //<NB> not available for a buffer in a bottom_extended
  // state.
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

void RSingleBuffer::cancel_charging()
{
  State::move_to(*this, undoingState);
  size_ = 0;
  // delete[] buf; buf = nullptr;
  State::move_to(*this, dischargedState);
}

void RSingleBuffer::extend_bottom(const RWindow& wnd)
{
  const size_t sz = wnd.filled_size();
  assert(sz > 0);
  SCHECK(sz <= bottom_reserved_);
  STATE(RSingleBuffer, move_to, bottom_extending);
  memcpy(buf + bottom_reserved_ - sz, wnd.cdata(), sz);
  size_ += sz;
  STATE(RSingleBuffer, move_to, bottom_extended);
}

}
