// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_TCPSOCKET_H_
#define CONCURRO_TCPSOCKET_H_

#include "RSocket.h"
#include "RObjectWithStates.h"
#include "StateMap.h"
#include "Logging.h"
#include <netdb.h>

DECLARE_AXIS(TCPAxis, SocketBaseAxis,
				 {//"created",
					//"closed",  
	 "in_closed",    // input part of connection is closed
	 "out_closed",
	 "listen",       // passive open
    "accepting",    // in the middle of a new ServerSocket
						  // creation
		//"ready",
	 "closing",      // fin-wait, time-wait, closing,
						  // close-wait, last-ack
	 //"aborted",   // see RFC793, 3.8 Abort
	 //"connection_timed_out",
	 //"connection_refused",
	 //"destination_unreachable"
	 },
  {
  {"created", "listen"},      // listen()
  {"listen", "accepting"}, // connect() from other side
  {"accepting", "listen"},
  {"listen", "closed"},
  {"created", "ready"}, // initial send() is
										 // recieved by other side
  {"created", "closed"},     // ask close() or timeout 
  {"ready", "closing"}, // our close() or FIN from
										// other side
  {"ready", "in_closed"},
  {"ready", "out_closed"},
  {"closing", "closed"},
  {"in_closed", "closed"},
  {"out_closed", "closed"},
  {"closed", "closed"}
  }
);

class TCPSocket : virtual public RSocketBase
, public RStateSplitter<TCPAxis, SocketBaseAxis>
{
  DECLARE_EVENT(TCPAxis, ready);
  DECLARE_EVENT(TCPAxis, closed);
  DECLARE_EVENT(TCPAxis, in_closed);
  DECLARE_EVENT(TCPAxis, out_closed);

public:
  DECLARE_STATES(TCPAxis, State);
  DECLARE_STATE_CONST(State, created);
  DECLARE_STATE_CONST(State, closed);
  DECLARE_STATE_CONST(State, in_closed);
  DECLARE_STATE_CONST(State, out_closed);
  DECLARE_STATE_CONST(State, listen);
  DECLARE_STATE_CONST(State, accepting);
  DECLARE_STATE_CONST(State, ready);
  DECLARE_STATE_CONST(State, closing);

  ~TCPSocket();

  /*CompoundEvent is_terminal_state() const
  {
	 return is_closed_event;
	 }*/

  //! Ask to close an outbound part
  void ask_close_out() override;

  std::string universal_id() const override
  {
	 return RSocketBase::universal_id();
  }

  void state_changed
	 (AbstractObjectWithStates* object) override;

  std::atomic<uint32_t>& 
	 current_state(const StateAxis& ax) override
  { 
	 return RStateSplitter<TCPAxis,SocketBaseAxis>
		::current_state(ax);
  }

  const std::atomic<uint32_t>& 
	 current_state(const StateAxis& ax) const override
  { 
	 return RStateSplitter<TCPAxis,SocketBaseAxis>
		::current_state(ax);
  }

#if 0
  Event get_event (const UniversalEvent& ue) override
  {
	 return RStateSplitter<TCPAxis,SocketBaseAxis>
		::get_event(ue);
  }

  Event get_event (const UniversalEvent& ue) const override
  {
	 return RStateSplitter<TCPAxis,SocketBaseAxis>
		::get_event(ue);
  }
#endif

  CompoundEvent create_event
	 (const UniversalEvent& ue) const override
  {
	 return RStateSplitter<TCPAxis,SocketBaseAxis>
		::create_event(ue);
  }

  void update_events
	 (TransitionId trans_id, uint32_t to) override
  {
	 LOG_TRACE(log, "update_events");
	 return RStateSplitter<TCPAxis,SocketBaseAxis>
		::update_events(trans_id, to);
  }

protected:
  typedef Logger<TCPSocket> log;
  
  //! It is set in the constructor by 
  //! ::getprotobyname("TCP") call
  struct protoent* tcp_protoent;
  
  //! Create a TCP socket in a "closed" state.
  TCPSocket
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
		  thread_name = SFORMAT("TCPSocket:" << sock->fd);
		}

		RThreadBase* create_derivation
		  (const ObjectCreationInfo& oi) const
		{ 
		  return new Thread(oi, *this); 
		}
	 };

  protected:
	 Thread(const ObjectCreationInfo& oi, const Par& p)
		: SocketThread(oi, p) {}
	 ~Thread() { destroy(); }
	 void run();
  }* thread;
};

#endif

