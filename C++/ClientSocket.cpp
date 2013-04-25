// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "ClientSocket.h"
#include "Event.h"
#include "TCPSocket.h" // TODO remove it

DEFINE_STATES(ClientSocketAxis, 
   {"created",
    "connecting",  
	 "connected",
	 "connection_timed_out",
	 "connection_refused", // got RST on SYN
	 "destination_unreachable",
	 "closed"
	 },
    { 
		{"created", "connecting"},
		{"connecting", "connected"},
		{"connecting", "connection_timed_out"},
		{"connecting", "connection_refused"},
		{"connecting", "destination_unreachable"},
		{"connected", "closed"}
	 }
);
DEFINE_STATE_CONST(ClientSocket, State, created);
DEFINE_STATE_CONST(ClientSocket, State, connecting);
DEFINE_STATE_CONST(ClientSocket, State, connected);
DEFINE_STATE_CONST(ClientSocket, State, 
						 connection_timed_out);
DEFINE_STATE_CONST(ClientSocket, State, 
						 connection_refused);
DEFINE_STATE_CONST(ClientSocket, State, 
						 destination_unreachable);
DEFINE_STATE_CONST(ClientSocket, State, closed);

ClientSocket::ClientSocket
  (const ObjectCreationInfo& oi, 
	const RSocketAddress& par)
  : 
	 RSocketBase(oi, par),
	 RObjectWithEvents<ClientSocketAxis>(createdState),
	 CONSTRUCT_EVENT(connecting),
	 CONSTRUCT_EVENT(connected),
	 CONSTRUCT_EVENT(connection_timed_out),
	 CONSTRUCT_EVENT(connection_refused),
	 CONSTRUCT_EVENT(destination_unreachable),
	 CONSTRUCT_EVENT(closed),

	 is_terminal_state_event {
		is_connection_timed_out_event,
		is_connection_refused_event,
		is_destination_unreachable_event,
		is_closed_event  
	 },

	 thread(dynamic_cast<Thread*>
			  (RSocketBase::repository->thread_factory
				-> create_thread(Thread::Par(this))))
{
  SCHECK(thread);
  this->RSocketBase::ancestor_terminals.push_back
	 (is_terminal_state());
  this->RSocketBase::ancestor_threads_terminals.push_back
	 (thread->is_terminated());
}

ClientSocket::~ClientSocket()
{
  //RSocketBase::is_terminal_state().wait();
  LOG_DEBUG(log, "~ClientSocket()");
}

void ClientSocket::ask_connect()
{
  ::connect
	 (fd, 
	  aw_ptr->begin()->ai_addr, 
	  aw_ptr->begin()->ai_addrlen);
  process_error(errno);
}

void ClientSocket::process_error(int error)
{
  switch (error) {
  case EINPROGRESS:
	 State::move_to(*this, connectingState);
	 // <NB> there are no connecting->connecting transition
	 thread->start();
	 return;
  case 0:
	 State::move_to(*this, connectedState);
	 return;
  case ETIMEDOUT:
	 State::move_to(*this, connection_timed_outState);
	 break;
  case ECONNREFUSED:
	 State::move_to(*this, connection_refusedState);
	 break;
  case ENETUNREACH:
	 State::move_to(*this, destination_unreachableState);
	 break;
  }
  RSocketBase::process_error(error);
}

void ClientSocket::Thread::run()
{
  ThreadState::move_to(*this, workingState);

  // TODO move to RSocket
  auto* tcp_sock = dynamic_cast<TCPSocket*>
	 (socket);
  SCHECK(tcp_sock);

  auto* cli_sock = dynamic_cast<ClientSocket*>
	 (socket);
  SCHECK(cli_sock);

  fd_set wfds;
  FD_ZERO(&wfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  // Wait for termination of a connection process
  FD_SET(fd, &wfds);
  rSocketCheck(
	 ::select(fd+1, NULL, &wfds, NULL, NULL) > 0);

  int error = 0;
  socklen_t error_len = sizeof(error);
  rSocketCheck(
	 getsockopt(fd, SOL_SOCKET, SO_ERROR, &error,
					&error_len) == 0);

  if (error)
	 cli_sock->process_error(error);

  (tcp_sock->is_in_closed()
	| tcp_sock->is_out_closed()
	| tcp_sock->is_closed()) . wait();

  if (tcp_sock->is_in_closed().signalled()) {
	 tcp_sock->ask_close_out();
	 ClientSocket::State::compare_and_move
		(*cli_sock, ClientSocket::connectedState, 
		 ClientSocket::closedState);
  }
  else if (tcp_sock->is_in_closed().signalled()) {
	 THROW_PROGRAM_ERROR;
  }
}


