// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 * A data buffer.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_RBUFFER_H_
#define CONCURRO_RBUFFER_H_

#include "RState.h"
#include "REvent.h"
#include "RObjectWithStates.h"
#include <list>

//! A data buffer states axis
DECLARE_AXIS(DataBufferStateAxis, StateAxis);

class RWindow;

/**
 * A data buffer.
 */
class RBuffer 
  : public RObjectWithEvents<DataBufferStateAxis>
{
  DECLARE_EVENT(DataBufferStateAxis, charged);
  DECLARE_EVENT(DataBufferStateAxis, discharged);

public:
  typedef RObjectWithEvents<DataBufferStateAxis> Parent;

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

  explicit RBuffer
    (const RState<DataBufferStateAxis>& initial_state);

  // temporary, to fix ticket:93
  RBuffer(const RBuffer&) = delete;

  virtual ~RBuffer();

  // temporary, to fix ticket:93
  RBuffer& operator=(const RBuffer&) = delete;

  CompoundEvent is_terminal_state() const override
  {
    return is_discharged_event;
  }

  //! Prepare the buffer to charging.
  virtual void start_charging() = 0;
  virtual void clear() = 0;

  //! Move the buffer
  virtual void move(RBuffer* from) = 0;

  std::string universal_id() const override
  {
    return "?";
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

class RSingleBuffer : public RBuffer
{
public:
  DEFINE_EXCEPTION(ResizeOverCapacity, 
                   "Can't resize RSingleBuffer "
                   "over its initial capacity");

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
  virtual ~RSingleBuffer();
 
  RSingleBuffer& operator=(const RSingleBuffer&) = delete;

  void move(RBuffer* from) override;

  //! Get available size of the buffer (top part only)
  size_t capacity() const { return top_reserved_; }
  //! Resize the buffer (bottom part is not change its size).
  void reserve(size_t res);
  //! Return the buffer data. Imply start_charging().
  void* data();
  //! Return the buffer data as a constant. Not imply
  //! start_charging().
  const void* cdata() const;
  //! Return used buffer size
  size_t size() const { return size_; }
  //! Mark the buffer as holding data. Also set filled.
  //! \throw ResizeOverCapacity
  void resize(size_t sz);
  void clear() { resize(0); }
  void start_charging();
  //! Append wnd below the buffer start. 
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

#endif


