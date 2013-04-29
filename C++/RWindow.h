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
#include "RSocketConnection.h"

class WindowAxis : public StateAxis {};

class RWindow
: public RObjectWithEvents<WindowAxis>
  //  public StdIdMember
{
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

  RWindow(RSingleSocketConnection* c);
  virtual ~RWindow();

  void forward_top(size_t);
  size_t size() const;

  const char& operator[](size_t) const;

  //! Skip all data
  void skip_rest();

  std::string universal_id() const
  {
	 return SFORMAT("RWindow:" << socket->fd);
  }
protected:
  DEFAULT_LOGGER(RWindow);

  class Thread : public SocketThread
  {
  public:
	 struct Par : public SocketThread::Par
	 {
		RWindow* win;

	   Par(RWindow* w) 
		  : SocketThread::Par(w->con->socket),
		  win(w)
		{
		  assert(win);
		  thread_name = SFORMAT("RWindow:" << socket->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new Thread(oi, *this); 
		}
	 };
  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p), win(p.win) {}
	 ~Thread() { destroy(); }
	 void run();

	 RWindow* win;
  };

  RSingleSocketConnection* con;
  InSocket* socket;
  RSingleBuffer buf;
  size_t bottom;
  size_t top;
  size_t sz;
  SocketThread* thread;

  //! It allows substitute a working thread from derived
  //! classes.
  RWindow(RSingleSocketConnection* c, bool no_thread);

  // <NB> not virtual
  void run();
};

#endif

