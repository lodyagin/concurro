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

#ifndef CONCURRO_RBUFFER_H_
#define CONCURRO_RBUFFER_H_

#include "RState.h"
#include "REvent.h"
#include "RObjectWithStates.h"
#include <list>

namespace curr {

/**
 * @addtogroup sockets
 * @{
 */

//! A data buffer states axis
DECLARE_AXIS(DataBufferStateAxis, StateAxis);

class RWindow;

/**
 * A data buffer interface.
 */
class RBuffer 
  : public RObjectWithEvents<DataBufferStateAxis>
{
  DECLARE_EVENT(DataBufferStateAxis, charged);
  DECLARE_EVENT(DataBufferStateAxis, discharged);

public:
  typedef RObjectWithEvents<DataBufferStateAxis> Parent;

  //! @cond
  DECLARE_STATES(DataBufferStateAxis, State);
  DECLARE_STATE_CONST(State, dummy);
  DECLARE_STATE_CONST(State, charging);
  DECLARE_STATE_CONST(State, charged);
  DECLARE_STATE_CONST(State, discharged);
  DECLARE_STATE_CONST(State, moving_destination);
  DECLARE_STATE_CONST(State, moving_source);
  DECLARE_STATE_CONST(State, resizing_discharged);
  DECLARE_STATE_CONST(State, bottom_extending);
  DECLARE_STATE_CONST(State, bottom_extended);
  DECLARE_STATE_CONST(State, undoing);
  //! @endcond

  explicit RBuffer
    (const RState<DataBufferStateAxis>& initial_state);

  //! A deleted constructor.
  // temporary, to fix ticket:93
  RBuffer(const RBuffer&) = delete;

  virtual ~RBuffer();

  //! A deleted assignment.
  // temporary, to fix ticket:93
  RBuffer& operator=(const RBuffer&) = delete;

  CompoundEvent is_terminal_state() const override
  {
    return is_discharged_event;
  }

  //! Prepare the buffer to charging.
  virtual void start_charging() = 0;
  //! Move state charged -> discharged
  virtual void clear() = 0;
  //! Cancel start_charging() 
  //! (charging -> undoing -> discharged).
  virtual void cancel_charging() = 0;

  //! Move the buffer
  virtual void move(RBuffer* from) = 0;

  std::string object_name() const override
  {
    return "RBuffer";
  }

  //! Autoclear means call clear() in the destructor
  void set_autoclear(bool autocl)
  {
    autoclear = autocl;
  }

  bool get_autoclear() const
  {
    return autoclear;
  }

  //! Return used buffer size
  virtual size_t size() const = 0;

protected:
  bool destructor_is_called;
  std::atomic<bool> autoclear;
};

/**
 * A "single" buffer implementation. When reading a stream
 * from a socket it allows extend_bottom(), i.e. append
 * top of the latest packet before the current packet.
 */
class RSingleBuffer : public RBuffer
{
  DECLARE_EVENT(DataBufferStateAxis, dummy);

public:
  DEFINE_EXCEPTION(ResizeOverCapacity, 
                   "Can't resize RSingleBuffer "
                   "over its initial capacity");

  //! Construct a buffer without reserved space. Need to
  //! call reserve() before using.
  // FIXME same state is different state
  RSingleBuffer();

  //! Construct a buffer with maximal size res +
  //! bottom_res. It will be possible append up to
  //! bottom_res bytes below the buffer start 
  //! (see extend_bottom()).
  explicit RSingleBuffer(size_t res, size_t bottom_res);
  //! Construct a buffer by moving a content from another
  //! buffer
  explicit RSingleBuffer(RBuffer* buf);
  RSingleBuffer(const RSingleBuffer& b) = delete;

  //! Always will wait "discharged" state. See
  //! set_autoclear(). 
  virtual ~RSingleBuffer();
 
  RSingleBuffer& operator=(const RSingleBuffer&) = delete;

  void move(RBuffer* from) override;

  //! Get available size of the buffer (top part only)
  size_t capacity() const { return top_reserved_; }
  //! Resize the buffer (top and bottom parts).
  void reserve(size_t top, size_t bottom);
  //! Return the buffer data. Imply start_charging().
  void* data();
  //! Return the buffer data as a constant. Not imply
  //! start_charging().
  const void* cdata() const;
  //! Return used buffer size
  size_t size() const override { return size_; }
  //! Mark the buffer as holding data. Also set filled.
  //! \throw ResizeOverCapacity
  void resize(size_t sz);
  void clear() override { resize(0); }
  void start_charging() override;
  void cancel_charging() override;
  //! Append data from wnd below the buffer start. 
  //! It will be accessible starting from the address
  //! (const char*) cdata() - wnd.size()
  void extend_bottom(const RWindow& wnd);

  DEFAULT_LOGGER(RSingleBuffer)

protected:
  //! Start of memory (bottom part)
  char* buf;
  size_t size_;
  //! A reserved memory size for a primary part
  size_t top_reserved_;
  //! A reserved memory size for a bottom part
  size_t bottom_reserved_;
};

#if 0
class RMultipleBuffer : public RBuffer
{
public:
  RMultipleBuffer();
protected:
  std::list<RSingleBuffer> bufs;
};
#endif

//! @}

}
#endif


