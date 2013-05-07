// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_CLIENTSOCKET_H_
#define CONCURRO_CLIENTSOCKET_H_

#include "RSocket.h"

DECLARE_AXIS(ClientSocketAxis, StateAxis,
   {"created",
    "connecting",  
	 "connected",
	 "connection_timed_out",
	 "connection_refused", // got RST on SYN
	 "destination_unreachable",
	 "closed"
	 },
    { 
		{"created", "closed"},
		{"created", "connecting"},
		{"connecting", "connected"},
		{"connecting", "connection_timed_out"},
		{"connecting", "connection_refused"},
		{"connecting", "destination_unreachable"},
		{"connected", "closed"}
	 }
);

class ClientSocket : virtual public RSocketBase,
  public RObjectWithEvents<ClientSocketAxis>
{
  DECLARE_EVENT(ClientSocketAxis, connecting)
  DECLARE_EVENT(ClientSocketAxis, connected)
  DECLARE_EVENT(ClientSocketAxis, connection_timed_out)
  DECLARE_EVENT(ClientSocketAxis, connection_refused)
  DECLARE_EVENT(ClientSocketAxis, destination_unreachable)
  DECLARE_EVENT(ClientSocketAxis, closed)

public:
  DECLARE_STATES(ClientSocketAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, connecting);
  DECLARE_STATE_CONST(State, connected);
  DECLARE_STATE_CONST(State, connection_timed_out);
  DECLARE_STATE_CONST(State, connection_refused);
  DECLARE_STATE_CONST(State, destination_unreachable);
  DECLARE_STATE_CONST(State, closed);

  const CompoundEvent is_terminal_state_event;

  CompoundEvent is_terminal_state() const override
  {
	 return is_terminal_state_event;
  }

  ~ClientSocket();

  //! Start connection to a server. It can result in
  //! connecting -> {connected, connection_timed_out,
  //! connection_refused and destination_unreachable}
  //! states.
  virtual void ask_connect();

  std::string universal_id() const
  {
	 return RSocketBase::universal_id();
  }

protected:
  ClientSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par);

  class Thread : public SocketThread
  {
  public:
	 struct Par : public SocketThread::Par
	 { 
		Par(RSocketBase* sock) 
		  : SocketThread::Par(sock) 
		{
		  thread_name = SFORMAT("ClientSocket:" << sock->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new Thread(oi, *this); 
		}
	 };

	 void run();
  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p) {}
	 ~Thread() { destroy(); }
  }* thread;

  DEFAULT_LOGGER(ClientSocket)

  void process_error(int error);
};

#endif
