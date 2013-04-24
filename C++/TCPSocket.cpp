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
//	 "syn_sent",
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
//  {"created", "syn_sent"},    // our connect()
  {"listen", "accepting"}, // connect() from other side
  {"accepting", "listen"},
  {"listen", "closed"},
  // {"listen", "syn_sent"},    // send()
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
  // TODO ::shutdown
  }
  );

DEFINE_STATE_CONST(TCPSocket, State, created);
DEFINE_STATE_CONST(TCPSocket, State, closed);
DEFINE_STATE_CONST(TCPSocket, State, in_closed);
DEFINE_STATE_CONST(TCPSocket, State, out_closed);
DEFINE_STATE_CONST(TCPSocket, State, listen);
//DEFINE_STATE_CONST(TCPSocket, State, syn_sent);
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
  //ask_close();
  //is_closed_event.wait();
  //RSocketBase::is_terminal_state().wait();
  LOG_DEBUG(log, "~TCPSocket()");
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

void TCPSocket::close_out()
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
		const ssize_t res = ::recv(fd, 0, 1, MSG_PEEK);
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

