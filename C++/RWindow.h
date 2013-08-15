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

DECLARE_AXIS(WindowAxis, StateAxis);

class RWindow
: public RObjectWithEvents<WindowAxis>,
  StdIdMember
{
  DECLARE_EVENT(WindowAxis, filled);
public:
  DECLARE_STATES(WindowAxis, State);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, filling);
  DECLARE_STATE_CONST(State, filled);
  DECLARE_STATE_CONST(State, welded);

  RWindow(const std::string& = std::string("RWindow"));
  RWindow(RWindow&);
  RWindow(RWindow&, 
          ssize_t shift_bottom, ssize_t shift_top);
  RWindow(RWindow&&);

  virtual ~RWindow() {}

  CompoundEvent is_terminal_state() const
  {
    //<NB> it is always terminal
    return CompoundEvent();
  }

  size_t size() const;

  RWindow& operator=(RWindow&); 
  RWindow& operator= (RWindow&&);

  virtual const char& operator[](size_t) const;

  std::string universal_id() const override
  {
    return universal_object_id;
  }

  //void resize(ssize_t shift_bottom, ssize_t shif_top);

protected:
  std::shared_ptr<RSingleBuffer> buf;
  size_t bottom;
  size_t top;
  size_t sz;

  DEFAULT_LOGGER(RWindow);
};

DECLARE_AXIS(ConnectedWindowAxis, WindowAxis);

//! A window which have live connection access.
class RConnectedWindow : public RWindow
{
  friend class RSingleSocketConnection;
  friend std::ostream&
    operator<<(std::ostream&, const RConnectedWindow&);

  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis, 
                  ready);
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis, 
                  filling);
  //A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis,
  //                filled);

public:
  struct Par {
    RSocketBase* socket;
    Par(RSocketBase* sock) : socket(sock) {}
    PAR_DEFAULT_VIRTUAL_MEMBERS(RConnectedWindow)
    SOCKET get_id(const ObjectCreationInfo& oi) const 
      { return socket->fd; }
  };

  RConnectedWindow(RSocketBase* sock);
  virtual ~RConnectedWindow();

  CompoundEvent is_terminal_state() const
  {
    return is_ready_event;
  }

  void forward_top(size_t);

protected:
  RConnectedWindow(const ObjectCreationInfo& oi,
                   const Par& par);

  //! A logic block reading implementation. It must set
  //! a filled state at the end. 
  virtual void move_forward();

  DEFAULT_LOGGER(RConnectedWindow);
};

std::ostream&
operator<<(std::ostream&, const RConnectedWindow&);

typedef Repository
<
RConnectedWindow,
  RConnectedWindow::Par,
  std::unordered_map,
  SOCKET
  >
  RConnectedWindowRepository;

#endif

