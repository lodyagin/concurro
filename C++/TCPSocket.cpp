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
  {"created", "closed"},     // ask close() or timeout 
  {"established", "closing"}, // our close() or FIN from
										// other side
  {"established", "in_closed"},
  {"established", "out_closed"},
  {"closing", "closed"},
  {"in_closed", "closed"},
  {"out_closed", "closed"},
  {"closed", "closed"}
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
  this->RSocketBase::threads_terminals.push_back
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
  LOG_DEBUG(log, "ask_close_out()");
  LOG_DEBUG(log, "shutdown(" << fd << ", SHUT_WR)");
  int res = ::shutdown(fd, SHUT_WR);
  if (res < 0) {
	 if (! errno == ENOTCONN) {
		rSocketCheck(false);
	 }
	 else {
		STATE(TCPSocket, move_to, closed);
		RSocketBase::State::compare_and_move
		  (*this, 
			{ RSocketBase::readyState,
			  RSocketBase::createdState
			}, 
			RSocketBase::closedState);
		return;
	 }
  }

  if (State::compare_and_move
		(*this, establishedState, out_closedState)
		||
		State::compare_and_move
		(*this, in_closedState, closedState)
	 )
	 RSocketBase::State::move_to
		(*this, RSocketBase::closedState);
}

void TCPSocket::Thread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();
  //ClientSocket* cli_sock = 0;
  //if (!(cli_sock = dynamic_cast<ClientSocket*>(socket))) 
  // THROW_NOT_IMPLEMENTED;
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*>(socket);
  assert(tcp_sock);
  
  ( socket->is_ready()
  | socket->is_closed()
  | socket->is_error()) . wait();

  if (socket->is_ready().signalled()) {
	 TCPSocket::State::move_to
		(*tcp_sock, establishedState);
  }
  else {
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
	 if (STATE_OBJ(TCPSocket, state_is, *tcp_sock, 
						in_closed)
		  || STATE_OBJ(TCPSocket, state_is, *tcp_sock,
							closed))
		FD_CLR(fd, &rfds);
	 else
		FD_SET(fd, &rfds);

	 /*if (State::state_is(*tcp_sock, out_closedState))
		FD_CLR(fd, &wfds);
	 else
	 FD_SET(fd, &wfds);*/

	 rSocketCheck(
		::select(fd+1, &rfds, /*&wfds,*/NULL, NULL, NULL) > 0);
	 LOG_DEBUG(log, "TCPSocket>\t ::select");

	 if (FD_ISSET(fd, &rfds)) {
		// peek the message size
		// Normally MSG_PEEK not need this buffer, but who
		// knows? 
		static char dummy_buf[1];
		const ssize_t res = ::recv
		  (fd, &dummy_buf, 1, MSG_PEEK);
		LOG_DEBUG(log, "TCPSocket>\t ::recv");
		if (res < 0 && errno == EAGAIN) 
		  continue; // no data available
		rSocketCheck(res >=0);
		assert(res >= 0);
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
		  {
			 RSocketBase::State::move_to
				(*socket, RSocketBase::closedState);
			 // TODO do not get out_closed yet
			 TCPSocket::State::compare_and_move
				(*tcp_sock, 
				 TCPSocket::in_closedState, 
				 TCPSocket::closedState);
			 break;
		  }
		}
		else {
		  if (RSocketBase::State::state_in
				(*socket, {RSocketBase::closedState,
					 RSocketBase::errorState})) {
			 // FIXME input message discarded here
			 TCPSocket::State::move_to
				(*tcp_sock, TCPSocket::closedState);
			 break;
		  }
		  else {
			 // peek other thread data, allow switch to it
#if 0
			 std::this_thread::yield();
#else
			 std::this_thread::sleep_for
				(std::chrono::milliseconds(100));
#endif
		  }
		}
	 }

	 //if (FD_ISSET(fd, &wfds)) {
		// FIXME need intercept SIGPIPE
	 //}
  }
}

