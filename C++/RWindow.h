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
#include "AutoRepository.h"
#include <iostream>

namespace curr {

/**
 * @defgroup connections
 * Connections.
 * @{
 */

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

  //! Clone a view to w's buffer.
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

  CompoundEvent is_terminal_state() const override
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
  //! shift from start of a buffer to the fist cell above
  //! the buffer
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
template<class ConnectionId>
class RConnectedWindow : public RWindow, public StdIdMember
{
  template<class ConnId>
  friend std::ostream&
    operator<<(std::ostream&, const RConnectedWindow<ConnId>&);

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

  struct Par {
    ConnectionId id;
    Par(const ConnectionId& conn_id) : id(conn_id) {}
    PAR_DEFAULT_VIRTUAL_MEMBERS(RConnectedWindow)
    ConnectionId get_id(const ObjectCreationInfo& oi) const 
      { return id; }
  };

#if 0
  //! Create RConnectedWindows with connection_id (it is
  //! used for logging).
  RConnectedWindow(int connection_id = 0);
#endif

  //! The copy constructor is deleted.
  RConnectedWindow(const RConnectedWindow&) = delete;

#if 0
  //! Construct a window which owns buf.
  //TODO move to RWindow
  explicit RConnectedWindow(RSingleBuffer* buf);
#endif

  //! Will wait till underlaying buffer is discharged.
  virtual ~RConnectedWindow();

  //! A delete assignment.
  RConnectedWindow& operator=
    (const RConnectedWindow&) = delete;

  std::string object_name() const override
  {
    return SFORMAT("RConnectedWindow:" << universal_id());
  }

  static RConnectedWindow<ConnectionId>* create
    (const ConnectionId& connection_id);

  CompoundEvent is_terminal_state() const override
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
  RConnectedWindow(const ObjectCreationInfo& oi,
                   const Par& par);

  //! Determine a data ready state. After return the state
  //! is wait_for_buffer or filled.
  virtual void move_forward();

  DEFAULT_LOGGER(RConnectedWindow);
};

template<class ConnectionId>
std::ostream&
operator<<(std::ostream&, 
           const RConnectedWindow<ConnectionId>&);


/**
 * 
 */
template<class ConnectionId>
using RConnectedWindowRepository =
  AutoRepository<RConnectedWindow<ConnectionId>, ConnectionId>;


/**
  * An RWindow as a streambuf.
  */
template <
  class CharT,
  class Traits = std::char_traits<CharT>
>
class RWindowStreambuf 
  : public std::basic_streambuf<CharT, Traits>,
    protected RWindow
{
public:
  RWindowStreambuf(RWindow&& w) 
  {
    RWindow::move(std::move(w));
    assert(state_is(*this, S(filled)));

    char* gbeg = const_cast<CharT*>
      (reinterpret_cast<const CharT*>(cdata()));

    setg(gbeg, gbeg, gbeg + filled_size() / sizeof(CharT));
  }

protected:
  std::streamsize showmanyc() override
  {
    return egptr() - gptr();
  }

  int_type underflow() override
  {
    return (showmanyc()) 
      ? Traits::to_int_type(*gptr()) 
      : Traits::eof();
  }

  pos_type seekoff
    ( 
      off_type off, 
      ios_base::seekdir dir,
      ios_base::openmode which = ios_base::in
     ) override
  {
    using namespace std;
    const pos_type end_pos = egptr() - eback();
    safe<off_type> abs_pos;

    switch(dir) {
      case ios_base::beg: 
        abs_pos = off;
        break;
      case ios_base::end:
        abs_pos = end_pos + off;
        break;
      case ios_base::cur:
        abs_pos = gptr() - eback() + off;
        break;
    }

    if (!(bool) abs_pos || abs_pos < 0) 
      // the rest will be checked in seekpos
      return pos_type(off_type(-1));
    
    return seekpos((off_type) abs_pos);
  }

  pos_type seekpos
    ( 
      pos_type pos, 
      ios_base::openmode which = ios_base::in
     ) override
  {
    const pos_type end_pos = egptr() - eback();

    if (pos > end_pos || which & ios_base::out)
      return pos_type(off_type(-1));

    setg(eback(), eback() + pos, egptr());
  }
              
private:
  typedef Logger<RWindowStreambuf> log;
};

//! @}

}
#endif

