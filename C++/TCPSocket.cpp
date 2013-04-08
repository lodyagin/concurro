// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "TCPSocket.h"

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
  {"reated", "syn_sent"},    // our connect()
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

TCPSocket::TCPSocket()
  : RObjectWithStates<TCPAxis> (closedState),
    tcp_protoent(NULL),
    thread(0)
{
  SCHECK((tcp_protoent = ::getprotobyname("TCP")) != NULL);
}

TCPSocket::~TCPSocket()
{
  ask_close();
  static REvent<TCPAxis>("closed").wait(*this);
  State::move_to(*this, destroyedState);
}

void TCPSocket::ask_close()
{
  State::move_to(*this, closingState);

  const int shutdown_ret = ::shutdown(fd, socket);
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

void TCPSocket::Thread::run()
{
  ClientSocket* cli_sock = 0;
  if (!(cli_sock = dynamic_cast<ClientSocket*>(this))) 
	 THROW_NOT_IMPLEMENTED;
  
  static REvent<ClientSocketStateAxis>("connecting").wait
	 (*this); // *this, not *cli_sock to check the API
  State::move_to(*this, syn_sentState);
  static REvent<ClientSocketStateAxis>("connected").wait
	 (*this);
  State::move_to(*this, establishedState);
}

