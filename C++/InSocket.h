// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_INSOCKET_H_
#define CONCURRO_INSOCKET_H_

#include "RSocketAddress.h"
#include "RSocket.h"
#include "RThread.h"
#include "RState.h"

class InSocketAxis : public StateAxis {};

class InSocket
: public RObjectWithEvents<InSocketAxis>,
  virtual public RSocketBase
{
  DECLARE_EVENT(InSocketAxis, new_data)
  DECLARE_EVENT(InSocketAxis, error)
  DECLARE_EVENT(InSocketAxis, closed)

public:
  DECLARE_STATES(InSocketAxis, State);
  DECLARE_STATE_CONST(State, new_data);
  DECLARE_STATE_CONST(State, error);
  DECLARE_STATE_CONST(State, empty);
  DECLARE_STATE_CONST(State, closed); // a reading side
                                      // was closed
  virtual void ask_close();

  CompoundEvent is_terminal_state() const
  {
	 return is_closed_event | is_error_event;
  }

  std::string universal_id() const
  {
	 return RSocketBase::universal_id();
  }

  //! The last received data
  RSingleBuffer msg;

protected:
 InSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par);

  class SelectThread : public SocketThreadWithPair
  {
  public:
	 struct Par : public SocketThreadWithPair::Par
	 { 
		Par(RSocketBase* sock) 
		  : SocketThreadWithPair::Par(sock) 
		{
		  thread_name = SFORMAT("InSocket(select):" 
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
		  thread_name = SFORMAT("InSocket(wait):" 
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

  DEFAULT_LOGGER(InSocket)

  //! Actual size of a socket internal read buffer + 1.
  size_t socket_rd_buf_size;
  SOCKET notify_fd;
  
  ~InSocket();
};

#endif
