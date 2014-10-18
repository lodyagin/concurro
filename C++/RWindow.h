/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
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

#include <iostream>
#include "InSocket.h"
#include "RBuffer.h"
#include "RState.h"
#include "RThread.h"
#include "RObjectWithStates.h"
#include "AutoRepository.h"
#include "types/safe.h"

namespace curr {

using namespace types;

/**
 * @defgroup connections
 * Connections.
 * @{
 */

DECLARE_AXIS(WindowAxis, MoveableAxis);

/**
 * A "view" to a part of RBuffer. One RBuffer can be
 * attached to several RWindow (see the copy assignment
 * and copy constructor).
 *
 *
 * - ready - not attached to RBuffer;
 * - filling - attached but data is updating (e.g., reading
 * from a socket);
 * - filled - attached and can manipulate data;
 *
 */
class RWindow 
  : public RObjectWithEvents<MoveableAxis, WindowAxis>
{
  friend class RSingleBuffer;

  A_DECLARE_EVENT(WindowAxis, MoveableAxis, ready);
  A_DECLARE_EVENT(WindowAxis, MoveableAxis, filled);

public:
  using Parent = RObjectWithEvents
    <MoveableAxis, WindowAxis>;

  //! @cond
  DECLARE_STATES(WindowAxis, State);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, filling);
  DECLARE_STATE_CONST(State, filled);
  DECLARE_STATE_CONST(State, moving_from_ready);
  //! @endcond

  //! Creates RWindow in "ready" state. 
  RWindow();

  //! Waits when w will be in "filled" state and connect to
  //! the same buffer.
  RWindow(RWindow& w);

  //! Clone w view to buffer with bottom and top shifts.
  RWindow(RWindow&, 
          ssize_t shift_bottom, ssize_t shift_top);

  //! A move constructor takes a buffer ownership from w.
  //! Waits when w will be in "filled" or "ready" state
  RWindow(RWindow&& w);

  //! Waits when w will be in "filled" state and connect to
  //! the same buffer.
  RWindow& operator=(RWindow& w);

  //! A move assignment.
  //! Waits when w will be in "filled" or "ready" state.
  RWindow& operator=(RWindow&& w);

  virtual ~RWindow() {}

  void swap(RWindow& o) = delete;

  std::string object_name() const override
  {
    return "RWindow";
  }

  //! Detach from a buffer if in filled state. <NB> Do
  //! nothing in other states.
  void detach();

#if 0
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
#endif

#if 0
  //! Move a buffer from w to this window. Old buffer will
  //! be detached.
  RWindow& partial_move(RWindow& w);
#endif

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

  CompoundEvent is_ready_or_filled_event = { 
    is_ready_event, is_filled_event
  };

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
 */
template<class ConnectionId>
class RConnectedWindow : public RWindow, public StdIdMember
{
  template<class ConnId>
  friend std::ostream&
  operator<<(std::ostream&, const RConnectedWindow<ConnId>&);

  A_DECLARE_EVENT(ConnectedWindowAxis, MoveableAxis, 
                  ready);
  A_DECLARE_EVENT(ConnectedWindowAxis, MoveableAxis, 
                  filling);
  A_DECLARE_EVENT(ConnectedWindowAxis, MoveableAxis,
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

  template<class T, int w>
  using Guard = NoGuard<T,w>;

  static constexpr int default_wait_m = 0;

  //! The copy constructor is deleted.
  RConnectedWindow(const RConnectedWindow&) = delete;

  //! Will wait till underlaying buffer is discharged.
  virtual ~RConnectedWindow();

  //! A delete assignment.
  RConnectedWindow& operator=
    (const RConnectedWindow&) = delete;

  std::string object_name() const override
  {
    return SFORMAT("RConnectedWindow:" << universal_id());
  }
  
  //! Create a new RConnectedWindow in
  //! RConnectedWindowRepository
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
class WindowStreambuf 
  : public std::basic_streambuf<CharT, Traits>,
    public RWindow
{
public:
  typedef typename Traits::int_type int_type;
  typedef typename Traits::pos_type pos_type;
  typedef typename Traits::off_type off_type;

  WindowStreambuf(RWindow&& w) 
  {
    RWindow::operator=(std::move(w));
    assert(state_is(*this, S(filled)));

    char* gbeg = const_cast<CharT*>
      (reinterpret_cast<const CharT*>(cdata()));

    this->setg
      (gbeg, gbeg, gbeg + filled_size() / sizeof(CharT));
  }

  // Returns a WindowStreambuf which contains subwindow
  // [ gptr(), gptr() + width )
  WindowStreambuf window(std::size_t width)
  {
    return WindowStreambuf
      (RWindow(*this, this->gptr() - this->eback(), 
               this->gptr() + width - this->egptr()));
  }

protected:
  std::streamsize showmanyc() override
  {
    return this->egptr() - this->gptr();
  }

  int_type underflow() override
  {
    return (showmanyc()) 
      ? Traits::to_int_type(*this->gptr()) 
      : Traits::eof();
  }

  pos_type seekoff
    ( 
      off_type off, 
      std::ios_base::seekdir dir,
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    using namespace std;
    const pos_type end_pos = this->egptr() - this->eback();
    safe<off_type> abs_pos(0);

    switch((uint32_t)dir) {
      case ios_base::beg: 
        abs_pos = off;
        break;
      case ios_base::end:
        abs_pos = end_pos + off;
        break;
      case ios_base::cur:
        abs_pos = this->gptr() - this->eback() + off;
        break;
    }

    if (!(bool) abs_pos || abs_pos < safe<off_type>(0)) 
      // the rest will be checked in seekpos
      return pos_type(off_type(-1));
    
    return seekpos((off_type) abs_pos);
  }

  pos_type seekpos
    ( 
      pos_type pos, 
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    const pos_type end_pos = this->egptr() - this->eback();

    if (pos > end_pos || which & std::ios_base::out)
      return pos_type(off_type(-1));

    this->setg
      (this->eback(), this->eback() + pos, this->egptr());
    return pos;
  }
              
private:
  typedef Logger<WindowStreambuf> log;
};

//! @}

}
#endif

