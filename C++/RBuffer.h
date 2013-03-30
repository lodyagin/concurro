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

//! Data buffer states axis
class DataBufferStateAxis : public StateAxis {};

/**
 * A data buffer.
 */
#if 0
class RBuffer
{
public:
  DECLARE_STATES(RBuffer, DataBufferStateAxis, State);
  DECLARE_STATE_CONST(State, charged);
  DECLARE_STATE_CONST(State, discharged);
  DECLARE_STATE_CONST(State, destroyed);

  //! Construct an empty buffer. Need to call reserve(res)
  //! after it.
  RBuffer();
  //! Construct a buffer with maximal size res.
  explicit RBuffer(size_t res);
  //! Move the buffer.
  RBuffer(RBuffer&& b);

  virtual ~RBuffer();

  //! Move the buffer.
  RBuffer& operator=(RBuffer&&);

  //! Get available size of the buffer
  size_t reserved() const { return reserved; }
  //! Resize the buffer.
  void reserve(size_t res);
  //! Return the buffer data
  void* data() { return buf; }
  //! Return used buffer size
  size_t size() const { return size; }
  //! Mark the buffer as holding data. Also set filled.
  void size(size_t sz) { size = sz; }

  //! It is set by size(sz) and reset by move the content
  //! by constuctor or assignment operator.
  REvent filled;

protected:
  void* buf;
  size_t size;
  const size_t reserved;
};
#endif

#endif


