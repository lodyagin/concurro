// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

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
  RWindow(const RWindow&) = delete;
  virtual ~RWindow() {}

  RWindow& operator=(const RWindow&) = delete;

  RWindow& attach_to(RWindow&);
  RWindow& attach_to(RWindow&, 
          ssize_t shift_bottom, ssize_t shift_top);
  RWindow& move(RWindow&);

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

  std::string universal_id() const override
  {
    return universal_object_id;
  }

protected:
  DEFAULT_LOGGER(RWindow);

  std::shared_ptr<RSingleBuffer> buf;
  ssize_t bottom;
  size_t top;
  size_t sz;
};

DECLARE_AXIS(ConnectedWindowAxis, WindowAxis);

//! A window which have live connection access.
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
  DECLARE_STATES(ConnectedWindowAxis, State);
  DECLARE_STATE_CONST(State, wait_for_buffer);

  struct Par {
    RSocketBase* socket;
    Par(RSocketBase* sock) : socket(sock) {}
    PAR_DEFAULT_VIRTUAL_MEMBERS(RConnectedWindow)
    SOCKET get_id(const ObjectCreationInfo& oi) const 
      { return socket->fd; }
  };

  RConnectedWindow(RSocketBase* sock);

  //! Construct a window which owns buf.
  //TODO move to RWindow
  //explicit RConnectedWindow(RSingleBuffer* buf);

  virtual ~RConnectedWindow();

  CompoundEvent is_terminal_state() const
  {
    return is_ready_event;
  }

  //! Increase the window and start waiting new data.
  //! Will transmit the object to the filling state. You
  //! should call is_filled().wait() after it.
  void forward_top(size_t);

  //! Append the new RSingleBuffer to the window. It
  //! should be new buffer (not simultaneously used by
  //! another class), e.g., a move copy of a socket
  //! buffer, see RSingleSocketConnection::run().
  void new_buffer(RSingleBuffer*);

protected:
  DEFAULT_LOGGER(RConnectedWindow);

  RConnectedWindow(const ObjectCreationInfo& oi,
                   const Par& par);

  //! A logic block reading implementation. It must set
  //! a filled state at the end. 
  virtual void move_forward();
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

