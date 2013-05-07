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

DEFINE_STATES(ClientSocketAxis);

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
  this->RSocketBase::threads_terminals.push_back
	 (thread->is_terminated());
  thread->start();
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
  socket->is_construction_complete_event.wait();

  // TODO move to RSocket
  auto* tcp_sock = dynamic_cast<TCPSocket*>
	 (socket);
  SCHECK(tcp_sock);

  auto* cli_sock = dynamic_cast<ClientSocket*>
	 (socket);
  SCHECK(cli_sock);

  ( cli_sock->is_connecting()
  | socket->is_closed()
  | socket->is_error()
  ) . wait();

  if (socket->is_closed().signalled()
		|| socket->is_error().signalled())
  {
	 ClientSocket::State::move_to
		(*cli_sock, ClientSocket::closedState);
	 return;
  }

  fd_set wfds;
  FD_ZERO(&wfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  // Wait for termination of a connection process
  FD_SET(fd, &wfds);
  rSocketCheck(
	 ::select(fd+1, NULL, &wfds, NULL, NULL) > 0);
  LOG_DEBUG(log, "ClientSocket>\t ::select");

  int error = 0;
  socklen_t error_len = sizeof(error);
  rSocketCheck(
	 getsockopt(fd, SOL_SOCKET, SO_ERROR, &error,
					&error_len) == 0);

  if (error)
	 cli_sock->process_error(error);

  if (!ClientSocket::State::compare_and_move
		(*cli_sock, 
		 ClientSocket::connectingState,
		 ClientSocket::connectedState))
	 return;

  // TODO move to RSocket
  RSocketBase::State::compare_and_move
	 (*socket, RSocketBase::createdState,
	  RSocketBase::readyState);

  (socket->is_closed() | socket->is_error()).wait();
  ClientSocket::State::compare_and_move
	 (*cli_sock, ClientSocket::connectedState, 
	  ClientSocket::closedState);
}


