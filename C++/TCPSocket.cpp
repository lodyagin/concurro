// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "TCPSocket.h"
#include "ClientSocket.h"

DEFINE_STATES(TCPAxis, 
   {"created",
    "closed",  
	 "in_closed",    // input part of connection is closed
	 "out_closed",
	 "listen",       // passive open
	 "syn_sent",
    "accepting",    // in the middle of a new ServerSocket
						  // creation
	 "established",
	 "closing",      // fin-wait, time-wait, closing,
						  // close-wait, last-ack
	 //"aborted",   // see RFC793, 3.8 Abort
	 "connection_timed_out",
	 "connection_refused",
	 "destination_unreachable",
	 "destroyed" // to check a state in the destructor
	 },
  {
  {"created", "listen"},      // listen()
  {"created", "syn_sent"},    // our connect()
  {"listen", "accepting"}, // connect() from other side
  {"accepting", "listen"},
  {"listen", "closed"},
  // {"listen", "syn_sent"},    // send()
  {"syn_sent", "established"}, // initial send() is
										 // recieved by other side
  {"syn_sent", "closed"},     // close() or timeout 
  {"established", "closing"}, // our close() or FIN from
										// other side
  {"established", "in_closed"},
  {"established", "out_closed"},
  {"closing", "closed"},
  {"in_closed", "closed"},
  {"out_closed", "closed"},
  {"closed", "destroyed"}
  // TODO ::shutdown
  }
  );

DEFINE_STATE_CONST(TCPSocket, State, created);
DEFINE_STATE_CONST(TCPSocket, State, closed);
DEFINE_STATE_CONST(TCPSocket, State, in_closed);
DEFINE_STATE_CONST(TCPSocket, State, out_closed);
DEFINE_STATE_CONST(TCPSocket, State, listen);
DEFINE_STATE_CONST(TCPSocket, State, syn_sent);
DEFINE_STATE_CONST(TCPSocket, State, accepting);
DEFINE_STATE_CONST(TCPSocket, State, established);
DEFINE_STATE_CONST(TCPSocket, State, closing);
DEFINE_STATE_CONST(TCPSocket, State, destroyed);

TCPSocket::TCPSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par)
  : 
	 RSocketBase(oi, par),
    RObjectWithEvents<TCPAxis> (closedState),
    tcp_protoent(NULL),
	 thread_factory(dynamic_cast<RSocketRepository*>
						 (oi.repository) -> thread_factory),
	 thread(dynamic_cast<Thread*>
			  (thread_factory->create_thread
				(Thread::Par(this))))
{
  SCHECK((tcp_protoent = ::getprotobyname("TCP")) !=
			NULL);
  thread->start();
}

TCPSocket::~TCPSocket()
{
  //ask_close();
  static REvent<TCPAxis> is_closed("closed");
  is_closed.wait(*this);
  assert(thread_factory);
  thread_factory->delete_thread(thread);
  State::move_to(*this, destroyedState);
}

#if 0
void TCPSocket::ask_close()
{
  State::move_to(*this, closingState);

  const int shutdown_ret = ::shutdown(fd, );
  //socketCreated.reset();
  State::move_to(*this, dest);
  if (close_ret == EWOULDBLOCK)
    ; // not all data was sent
  // see http://alas.matf.bg.ac.rs/manuals/lspe/snode=105.html
  else if (close_ret == 0)
    State::move_to(*this, closedState); // SO_LINGER guarantees it
  else 
    rSocketCheck(false); // another socket error

  if (!State::state_is(*this, closedState))
    State::move_to(*this, closedState);
}
#endif

void TCPSocket::Thread::run()
{
  ClientSocket* cli_sock = 0;
  if (!(cli_sock = dynamic_cast<ClientSocket*>(socket))) 
	 THROW_NOT_IMPLEMENTED;
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*>(socket);
  assert(tcp_sock);
  
  static REvent<ClientSocketAxis> 
	 is_connecting("connecting");
  is_connecting.wait(*cli_sock);
  TCPSocket::State::move_to(*tcp_sock, syn_sentState);
  static REvent<ClientSocketAxis> 
	 is_connected("connected");
  is_connected.wait(*cli_sock);
  TCPSocket::State::move_to(*tcp_sock, establishedState);
}

