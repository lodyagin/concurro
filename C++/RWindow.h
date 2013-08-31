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

#ifndef CONCURRO_RWINDOW_H_
#define CONCURRO_RWINDOW_H_

#include "InSocket.h"
#include "RBuffer.h"
#include "RState.h"
#include "RThread.h"
#include "RObjectWithStates.h"

namespace curr {

DECLARE_AXIS(WindowAxis, StateAxis);

/**
 * A "view" to a part of RBuffer. One RBuffer can be
 * attached to several RWindow (see attach_to()).
 *
 * @dot
 * digraph {
 *    start [shape = point]; 
 *    stop [shape = point];
 *    start -> ready;
 *    ready -> stop;
 *    ready -> filling [label = "attach_to()"];
 *    filling -> filled [label = "attach_to()"];
 *    filled -> welded;
 *    welded -> filled;
 *    welded -> ready [label = "detach()"];
 * }
 * @enddot
 *
 * - ready - not attached to RBuffer;
 * - filling - attached but data is updating (e.g., reading
 * from a socket);
 * - filled - attached and can manipulate data;
 * - welded - share window with another buffer (usually
 * temporary, see methods). 
 *
 */
class RWindow : public RObjectWithEvents<WindowAxis>
{
  friend class RSingleBuffer;

  DECLARE_EVENT(WindowAxis, filled);

public:
  //! @cond
  DECLARE_STATES(WindowAxis, State);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, filling);
  DECLARE_STATE_CONST(State, filled);
  DECLARE_STATE_CONST(State, welded);
  //! @endcond

  //! Create RWindow in "ready" state. 
  RWindow();

  //! Clone w view to buffer.
  RWindow(RWindow& w);

  //! Clone w view to buffer with bottom and top shifts.
  RWindow(RWindow&, 
          ssize_t shift_bottom, ssize_t shift_top);

  //! A move constructor takes a buffer ownership from w.
  RWindow(RWindow&& w);

  virtual ~RWindow();

  //! A deleted assignment.
  RWindow& operator=(const RWindow&) = delete;

  //! A move assignment.
  RWindow& operator=(RWindow&& w);

  std::string object_name() const override
  {
    return "RWindow";
  }

  //! Detach from a buffer if in filled state. <NB> Do
  //! nothing in other states.
  void detach();

  //! Wait when w will be in "filled" state and connect to
  //! the same buffer.
  RWindow& attach_to(RWindow& w);

  //! Wait when w will be in "filled" state and connect to
  //! the same buffer. This window will make view to the
  //! buffer shifted by
  //! shift_bottom and shift_top relatively to w. It can
  //! be shifted only inside filled region.
  RWindow& attach_to(RWindow& w, 
          ssize_t shift_bottom, ssize_t shift_top);

  //! Move a buffer from w to this window. Old buffer will
  //! be detached.
  RWindow& move(RWindow& w);

  CompoundEvent is_terminal_state() const
  {
    //<NB> it is always terminal
    return CompoundEvent();
  }

  //! In a state "filling" it is wanted size requested by
  //! forward_top(sz). 
  size_t size() const;

  //! A size of a part already filled with a data.
  size_t filled_size() const
  { return top - bottom; }

  virtual const char& operator[](size_t) const;

protected:
  const char* cdata() const;

  std::shared_ptr<RSingleBuffer> buf;
  ssize_t bottom;
  ssize_t top;
  size_t sz;

  DEFAULT_LOGGER(RWindow);
};

DECLARE_AXIS(ConnectedWindowAxis, WindowAxis);

/**
 * A window which have a live connection access (typically
 * RSocketConnection). A working thread from connection
 * side will wait "wait_for_buffer" state and new packet
 * arrival (in a form of RSingleBuffer). After the the
 * working thread will call new_buffer(). I.e.,
 * RConnectedWindow is a passive entity and
 * RSocketConnection is an active one.
 *
 * @dot
 * digraph {
 *    start [shape = point]; 
 *    stop [shape = point];
 *    ready [label = "ready"];
 *    start -> ready;
 *    ready -> stop;
 *    ready -> filling [label = "forward_top(sz)"];
 *    filling -> wait_for_buffer;
 *    wait_for_buffer -> filling [label="new_buffer()"];
 *    filling -> filled;
 *    filled -> welded;
 *    welded -> filled;
 *    welded -> ready;
 * }
 * @enddot
 */
class RConnectedWindow : public RWindow
{
  friend std::ostream&
    operator<<(std::ostream&, const RConnectedWindow&);

  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis, 
                  ready);
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis, 
                  filling);
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis,
                  wait_for_buffer);
public:
  //! @cond
  DECLARE_STATES(ConnectedWindowAxis, State);
  DECLARE_STATE_CONST(State, wait_for_buffer);
  //! @endcond

  //! Create RConnectedWindows with connection_id (it is
  //! used for logging).
  RConnectedWindow(const std::string& connection_id 
                   = std::string());

  //! Construct a window which owns buf.
  //TODO move to RWindow
  //explicit RConnectedWindow(RSingleBuffer* buf);

  //! Will wait till underlaying buffer is discharged.
  virtual ~RConnectedWindow();

  std::string object_name() const override
  {
    return SFORMAT("RConnectedWindow");
  }

  CompoundEvent is_terminal_state() const
  {
    return is_ready_event;
  }

  //! Increase the window and start waiting new data.
  //! Will move the object to the wait_for_buffer
  //! state. You should call is_filled().wait() after it.
  void forward_top(size_t);

  //! Append the new RSingleBuffer to the window. It
  //! should be new buffer (not simultaneously used by
  //! another class), e.g., a move copy of a socket
  //! buffer, see RSingleSocketConnection::run().
  void new_buffer(std::unique_ptr<RSingleBuffer>&&);

protected:
  const std::string conn_id;

  //! Determine a data ready state. After return the state
  //! is wait_for_buffer or filled.
  virtual void move_forward();

  DEFAULT_LOGGER(RConnectedWindow);
};

std::ostream&
operator<<(std::ostream&, const RConnectedWindow&);

}
#endif

