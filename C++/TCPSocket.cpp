// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "TCPSocket.h"

// RSocket states  ========================================

DEFINE_STATES(TCPSocket, TCPStateAxis, State)
(  {"closed",  
	 "listen",       // passive open
	 "syn-sent",
//   "syn-rcvd",
	 "established",
	 "closing",      // fin-wait, time-wait, closing, close-wait, last-ack
	 "aborted",   // see RFC793, 3.8 Abort
	 "destroyed" // to check a state in the destructor
	 },
  {
  {"closed", "listen"},      // listen()
  {"closed", "syn-sent"},    // our connect()
  {"listen", "established"}, // connect() from other side
  {"listen", "syn-sent"},    // send()
  {"syn-sent", "established"}, // initial send() is resived by other side
  {"syn-sent", "closed"},     // close() or timeout 
  {"established", "closing"}, // our close() or FIN from other side
  {"closing", "closed"},
  {"closed", "destroyed"}
  // TODO ::shutdown
  }
  );

DEFINE_STATE_CONST(TCPSocket, State, closed);
DEFINE_STATE_CONST(TCPSocket, State, listen);
DEFINE_STATE_CONST(TCPSocket, State, syn_sent);
DEFINE_STATE_CONST(TCPSocket, State, established);
DEFINE_STATE_CONST(TCPSocket, State, closing);
DEFINE_STATE_CONST(TCPSocket, State, aborted);
DEFINE_STATE_CONST(TCPSocket, State, destroyed);

TCPSocket::TCPSocket(int close_wait_seconds)
  : RObjectWithStates<TCPStateAxis> (closedState),
    tcp_protoent(NULL), 
    close_wait_secs(close_wait_seconds)
{
  SCHECK((tcp_protoent = ::getprotobyname("TCP")) != NULL);
  
  // TCPSocket semantic doesn't allow reset connection, 
  // please use another class for it
  SCHECK(close_wait_secs > 0);
}

TCPSocket::~TCPSocket()
{
  close();
  State::move_to(*this, destroyedState);
}

void TCPSocket::close()
{
  const State dest = 
	 (State::state_is(*this, syn_sentState))
	 ? closedState : closingState;
  State::check_moving_to(*this, dest);

  // these steps are needed to send all data
  // see http://ia700609.us.archive.org/22/items/TheUltimateSo_lingerPageOrWhyIsMyTcpNotReliable/the-ultimate-so_linger-page-or-why-is-my-tcp-not-reliable.html
  const bool saved_state = get_blocking();
  set_blocking(true); 
  struct linger linger;
  linger.l_onoff = 1; // on
  linger.l_linger = close_wait_secs;
  rSocketCheck(
    (::setsockopt(socket, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)))
    != -1);
  set_blocking(saved_state);
  const int close_ret = ::close(socket);
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

#if 0
void TCPSocket::connect_first (const RClientSocketAddress& addr)
{
  State::check_moving_to(*this, syn_sentState);

  for (auto cit = addr.begin (); cit != addr.end (); cit++)
  {
	 if (socket != INVALID_SOCKET)
		rSocketCheck(::close(socket) == 0);
#if 1
	 socket
#else
	 rSocketCheck(
		(socket = ::socket
       (cit->ai_family, cit->ai_socktype, cit->ai_protocol)) 
		!= INVALID_SOCKET
		);
#endif
	 //socketCreated.set();

    LOG_DEBUG(log, "Connecting to " << *cit);
	 rSocketCheck(::connect(socket, cit->ai_addr, cit->ai_addrlen) == 0);
	 break; // FIXME - continue with the next address
  }

  if (get_blocking()) {
	 State::move_to(*this, syn_sentState);
	 State::move_to(*this, establishedState);
  }
  else {
	 State::move_to(*this, syn_sentState);
  }
}
#endif


