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

  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis, 
						filling);
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis,
						filled);
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis,
						skipping);
  A_DECLARE_EVENT(ConnectedWindowAxis, WindowAxis,
						destroyed);

public:
  DECLARE_STATES(ConnectedWindowAxis, State);
  DECLARE_STATE_CONST(State, skipping);
  DECLARE_STATE_CONST(State, destroyed);

  RConnectedWindow(const std::string& id);
  virtual ~RConnectedWindow();

  void forward_top(size_t);

  const char& operator[](size_t) const override;

  //! Skip all data
  void skip_rest();

protected:
  DEFAULT_LOGGER(RConnectedWindow);

  //! A logic block reading implementation. It must set
  //! a filled state at the end. 
  virtual void move_forward();
};

#endif

