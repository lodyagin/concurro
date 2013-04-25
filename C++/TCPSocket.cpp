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
    "accepting",    // in the middle of a new ServerSocket
						  // creation
	 "established",
	 "closing",      // fin-wait, time-wait, closing,
						  // close-wait, last-ack
	 //"aborted",   // see RFC793, 3.8 Abort
	 "connection_timed_out",
	 "connection_refused",
	 "destination_unreachable"
	 },
  {
  {"created", "listen"},      // listen()
  {"listen", "accepting"}, // connect() from other side
  {"accepting", "listen"},
  {"listen", "closed"},
  {"created", "established"}, // initial send() is
										 // recieved by other side
  {"created", "closed"},     // close() or timeout 
  {"established", "closing"}, // our close() or FIN from
										// other side
  {"established", "in_closed"},
  {"established", "out_closed"},
  {"closing", "closed"},
  {"in_closed", "closed"},
  {"out_closed", "closed"}
  }
  );

DEFINE_STATE_CONST(TCPSocket, State, created);
DEFINE_STATE_CONST(TCPSocket, State, closed);
DEFINE_STATE_CONST(TCPSocket, State, in_closed);
DEFINE_STATE_CONST(TCPSocket, State, out_closed);
DEFINE_STATE_CONST(TCPSocket, State, listen);
DEFINE_STATE_CONST(TCPSocket, State, accepting);
DEFINE_STATE_CONST(TCPSocket, State, established);
DEFINE_STATE_CONST(TCPSocket, State, closing);

TCPSocket::TCPSocket
	 (const ObjectCreationInfo& oi, 
	  const RSocketAddress& par)
  : 
	 RSocketBase(oi, par),
    RObjectWithEvents<TCPAxis> (createdState),
	 CONSTRUCT_EVENT(established),
	 CONSTRUCT_EVENT(closed),
	 CONSTRUCT_EVENT(in_closed),
	 CONSTRUCT_EVENT(out_closed),
    tcp_protoent(NULL),
	 thread(dynamic_cast<Thread*>
			  (RSocketBase::repository->thread_factory
				-> create_thread(Thread::Par(this))))
{
  SCHECK(thread);
  this->RSocketBase::ancestor_terminals.push_back
	 (is_terminal_state());
  this->RSocketBase::ancestor_threads_terminals.push_back
	 (thread->is_terminated());
  SCHECK((tcp_protoent = ::getprotobyname("TCP")) !=
			NULL);
  thread->start();
}

TCPSocket::~TCPSocket()
{
  LOG_DEBUG(log, "~TCPSocket()");
}

void TCPSocket::ask_close_out()
{
  rSocketCheck(
	 ::shutdown(fd, SHUT_WR) == 0);

  State::compare_and_move
	 (*this, establishedState, out_closedState)
	 ||
  State::compare_and_move
	 (*this, in_closedState, closedState);
}

void TCPSocket::Thread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();
  ClientSocket* cli_sock = 0;
  if (!(cli_sock = dynamic_cast<ClientSocket*>(socket))) 
	 THROW_NOT_IMPLEMENTED;
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*>(socket);
  assert(tcp_sock);
  
//  cli_sock->is_connecting().wait_shadow();
//  TCPSocket::State::move_to(*tcp_sock, syn_sentState);

  (cli_sock->is_terminal_state_event 
	| cli_sock->is_connected()) . wait(); 
  //TODO is_connected as a shadow

  if (cli_sock->is_connected().signalled()) {
	 TCPSocket::State::move_to
		(*tcp_sock, establishedState);
  }
  else if (cli_sock->is_connection_refused().signalled()){
	 TCPSocket::State::move_to
		(*tcp_sock, closedState);
	 return;
  }
  
  // wait for close
  fd_set wfds, rfds;
  FD_ZERO(&wfds);
  FD_ZERO(&rfds);
  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  for (;;) {
	 if (TCPSocket::State::state_is
		  (*tcp_sock, TCPSocket::in_closedState))
		FD_CLR(fd, &rfds);
	 else
		FD_SET(fd, &rfds);

	 /*if (State::state_is(*tcp_sock, out_closedState))
		FD_CLR(fd, &wfds);
	 else
	 FD_SET(fd, &wfds);*/

	 rSocketCheck(
		::select(fd+1, &rfds, /*&wfds,*/NULL, NULL, NULL) > 0);

	 if (FD_ISSET(fd, &rfds)) {
		// peek the message size
		ssize_t res;
		// Normally MSG_PEEK not need this buffer, but who
		// knows? 
		static char dummy_buf[1];
		rSocketCheck(
		  (res = ::recv(fd, &dummy_buf, 1, MSG_PEEK) >=0));
		SCHECK(res >= 0);
		if (res == 0) {
		  if (TCPSocket::State::compare_and_move
				(*tcp_sock, 
				 TCPSocket::establishedState, 
				 TCPSocket::in_closedState
				  )
				|| 
				TCPSocket::State::compare_and_move
				(*tcp_sock, 
				 TCPSocket::out_closedState, 
				 TCPSocket::closedState))
			 break;
		}
	 }

	 //if (FD_ISSET(fd, &wfds)) {
		// FIXME need intercept SIGPIPE
	 //}
  }
}

