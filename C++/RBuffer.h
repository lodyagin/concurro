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
DECLARE_AXIS(DataBufferStateAxis, StateAxis,
  {   "charging", // is locked for filling
      "charged", // has data
		"discharged", // data was red (moved)
		"welded"  // own a buffer together with another
					 // RBuffer
		},
  { {"discharged", "charging"},
	 {"charging", "charged"},
		//{"charging", "discharged"}, 
		// there is a week control if its enabled
	 {"discharged", "welded"},
	 {"welded", "discharged"},
	 {"welded", "charged"},
    {"charged", "discharged"}}
);


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

  CompoundEvent is_terminal_state() const
  {
	 return is_discharged_event;
  }

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

  //! Autoclear means call clear() in the destructor
  void set_autoclear(bool autocl)
  {
	 autoclear = autocl;
  }

  bool get_autoclear() const
  {
	 return autoclear;
  }

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


