// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_OUTSOCKET_H_
#define CONCURRO_OUTSOCKET_H_

#include "RSocketAddress.h"
#include "RSocket.h"
#include "RThread.h"
#include "RState.h"

class OutSocketAxis : public StateAxis {};

class OutSocket
: public RObjectWithEvents<OutSocketAxis>,
  virtual public RSocketBase
{
  DECLARE_EVENT(OutSocketAxis, wait_you)
  DECLARE_EVENT(OutSocketAxis, error)
  DECLARE_EVENT(OutSocketAxis, closed)

public:
  DECLARE_STATES(OutSocketAxis, State);
  DECLARE_STATE_CONST(State, wait_you);
  DECLARE_STATE_CONST(State, busy);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, error);

  CompoundEvent is_terminal_state() const
  {
	 return is_closed_event | is_error_event;
  }

  std::string universal_id() const
  {
	 return RSocketBase::universal_id();
  }

  //! A buffer to send data
  RSingleBuffer msg;

protected:
  OutSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par);
  ~OutSocket();

  class SelectThread : public SocketThreadWithPair
  {
  public:
	 struct Par : public SocketThreadWithPair::Par
	 { 
		Par(RSocketBase* sock) 
		  : SocketThreadWithPair::Par(sock) 
		{
		  thread_name = SFORMAT("OutSocket(select):" 
										<< sock->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new SelectThread(oi, *this); 
		}
	 };

	 void run();

  protected:
	 SelectThread
		(const ObjectCreationInfo& oi, const Par& p)
		: SocketThreadWithPair(oi, p) {}

	 ~SelectThread() { destroy(); }
  }* select_thread;

  class WaitThread : public SocketThread
  {
  public:
	 struct Par : public SocketThread::Par
	 { 
		SOCKET notify_fd;
	   Par(RSocketBase* sock, SOCKET notify) 
		: SocketThread::Par(sock),
		  notify_fd(notify)
		{
		  thread_name = SFORMAT("OutSocket(wait):" 
										<< sock->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new WaitThread(oi, *this); 
		}
	 };

	 void run();

  protected:
	 SOCKET notify_fd;

	 WaitThread
		(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p), notify_fd(p.notify_fd) {}
	 ~WaitThread() { destroy(); }
  }* wait_thread;

  DEFAULT_LOGGER(OutSocket)

  SOCKET notify_fd;
};


#endif
