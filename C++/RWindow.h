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
public:
  DECLARE_STATES(WindowAxis, State);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, filling);
  DECLARE_STATE_CONST(State, filled);
  DECLARE_STATE_CONST(State, welded);

  RWindow(const std::string& = std::string("RWindow"));
  RWindow(RWindow&);
  RWindow(RWindow&&);

  virtual ~RWindow() {}

  size_t size() const;

  RWindow& operator= (RWindow&);
  RWindow& operator= (RWindow&&);

  virtual const char& operator[](size_t) const;

  std::string universal_id() const override
  {
	 return universal_object_id;
  }

protected:
  DEFAULT_LOGGER(RWindow);

  std::shared_ptr<RSingleBuffer> buf;
  size_t bottom;
  size_t top;
  size_t sz;
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
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis,
						filled);

public:
  struct Par {
	 RSocketBase* socket;
    Par(RSocketBase* sock) : socket(sock) {}
	 PAR_DEFAULT_VIRTUAL_MEMBERS(RConnectedWindow)
	 SOCKET get_id(const ObjectCreationInfo& oi) const 
	 { return socket->fd; }
  };

  //DECLARE_STATES(ConnectedWindowAxis, State);
  //DECLARE_STATE_CONST(State, destroyed);

  RConnectedWindow(RSocketBase* sock);
  virtual ~RConnectedWindow();

  void forward_top(size_t);

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

