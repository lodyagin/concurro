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
//#include "RSocketConnection.h"

DECLARE_AXIS(WindowAxis, StateAxis);

class RWindow
: public RObjectWithEvents<WindowAxis>,
  StdIdMember
{
  friend class RSingleSocketConnection;

  DECLARE_EVENT(WindowAxis, filling);
  DECLARE_EVENT(WindowAxis, filled);
  DECLARE_EVENT(WindowAxis, skipping);
  DECLARE_EVENT(WindowAxis, destroyed);

public:
  DECLARE_STATES(WindowAxis, State);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, filling);
  DECLARE_STATE_CONST(State, filled);
  DECLARE_STATE_CONST(State, skipping);
  DECLARE_STATE_CONST(State, destroyed);

  RWindow(const std::string& id);
//  RWindow(RSingleSocketConnection* c);
  virtual ~RWindow();

  void forward_top(size_t);
  size_t size() const;

  virtual const char& operator[](size_t) const;

  //! Skip all data
  void skip_rest();

  std::string universal_id() const
  {
	 return universal_object_id;
  }

protected:
  DEFAULT_LOGGER(RWindow);

  //RSingleSocketConnection* con;
  //InSocket* socket;
  std::shared_ptr<RSingleBuffer> buf;
  size_t bottom;
  size_t top;
  size_t sz;
  //SocketThread* thread;

  //! It allows substitute a working thread from derived
  //! classes.
  //RWindow(RSingleSocketConnection* c, bool no_thread);

  // <NB> not virtual
  //void run();

  // A logic block reading implementation. It must set
  // a filled state at the end. 
  virtual void move_forward();
};

#endif

