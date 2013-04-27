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
class DataBufferStateAxis : public StateAxis {};

/**
 * A data buffer.
 */
class RBuffer 
  : public RObjectWithEvents<DataBufferStateAxis>
{
  DECLARE_EVENT(DataBufferStateAxis, charged);
  DECLARE_EVENT(DataBufferStateAxis, discharged);

public:
  DECLARE_STATES(DataBufferStateAxis, State);
  DECLARE_STATE_CONST(State, charging);
  DECLARE_STATE_CONST(State, charged);
  DECLARE_STATE_CONST(State, discharged);
  DECLARE_STATE_CONST(State, welded);

  RBuffer();

  //! Move the buffer.
  RBuffer(RBuffer&& b);
  virtual ~RBuffer();

  RBuffer& operator=(RBuffer&& b);

  //! Prepare the buffer to charging.
  virtual void start_charging() = 0;
  virtual void clear() = 0;

  // to make this class abstract
  //virtual void dummy() = 0;

  //! Move the buffer.
  //RBuffer& operator=(RBuffer&&);

  std::string universal_id() const
  {
	 return "?";
  }

protected:
  bool destructor_is_called;
};

class RSingleBuffer : public RBuffer
{
public:
  DEFINE_EXCEPTION(ResizeOverCapacity, 
						 "Can't resize RSingleBuffer "
						 "over its initial capacity");

  RSingleBuffer();
  //! Construct a buffer with maximal size res.
  explicit RSingleBuffer(size_t res);
  RSingleBuffer(const RSingleBuffer& b) = delete;
  //! Move the buffer.
  RSingleBuffer(RSingleBuffer&& b);
  virtual ~RSingleBuffer();
 
  RSingleBuffer& operator=(const RSingleBuffer&) = delete;
  //! Move the buffer.
  RSingleBuffer& operator=(RSingleBuffer&&);

  //! Get available size of the buffer
  size_t capacity() const { return reserved_; }
  //! Resize the buffer.
  void reserve(size_t res);
  //! Return the buffer data. Imply start_charging().
  void* data();
  //! Return the buffer data as a constant. Not imply
  //! start_charging().
  const void* cdata() const { return buf; }
  //! Return used buffer size
  size_t size() const { return size_; }
  //! Mark the buffer as holding data. Also set filled.
  //! \throw ResizeOverCapacity
  void resize(size_t sz);
  void clear() { resize(0); }
  void start_charging();

  DEFAULT_LOGGER(RSingleBuffer)

protected:
  char* buf;
  size_t size_;
  size_t reserved_;
};

class RMultipleBuffer : public RBuffer
{
public:
  RMultipleBuffer();
protected:
  std::list<RSingleBuffer> bufs;
};

#endif


