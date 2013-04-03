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
public:
  DECLARE_STATES(DataBufferStateAxis, State);
  DECLARE_STATE_CONST(State, charged);
  DECLARE_STATE_CONST(State, discharged);
  DECLARE_STATE_CONST(State, destroyed);

  RBuffer() 
  : RObjectWithEvents<DataBufferStateAxis>
	 (dischargedState) {}

  //! Move the buffer.
  //RBuffer(RBuffer&& b);
  virtual ~RBuffer();

  // to make this class abstract
  //virtual void dummy() = 0;

  //! Move the buffer.
  //RBuffer& operator=(RBuffer&&);

};

class RSingleBuffer : public RBuffer
{
public:
  DEF_EXCEPTION(ResizeOverCapacity, 
					 "Can't resize RSingleBuffer "
					 "over its initial capacity");

  RSingleBuffer() = delete;
  //! Construct a buffer with maximal size res.
  explicit RSingleBuffer(size_t res);
  //! Move the buffer.
  RSingleBuffer(RSingleBuffer&& b);
  virtual ~RSingleBuffer();
 
  //! Move the buffer.
  RBuffer& operator=(RBuffer&&);

  //! Get available size of the buffer
  size_t capacity() const { return reserved_; }
  //! Resize the buffer.
  //void reserve(size_t res);
  //! Return the buffer data
  void* data() { return buf; }
  //! Return used buffer size
  size_t size() const { return size_; }
  //! Mark the buffer as holding data. Also set filled.
  //! \throw ResizeOverCapacity
  void resize(size_t sz);

  DEFAULT_LOGGER(RSingleBuffer)

protected:
  char* buf;
  size_t size_;
  const size_t reserved_;
};

class RMultipleBuffer : public RBuffer
{
public:
  RMultipleBuffer();
protected:
  std::list<RSingleBuffer> bufs;
};

#endif


