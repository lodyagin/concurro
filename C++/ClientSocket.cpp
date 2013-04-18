// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "ClientSocket.h"
#include "Event.h"

DEFINE_STATES(ClientSocketAxis, 
   {"created",
    "connecting",  
	 "connected",
	 "connection_timed_out",
	 "connection_refused", // got RST on SYN
	 "destination_unreachable"
	 },
    { 
		{"created", "connecting"},
		{"connecting", "connected"},
		{"connecting", "connection_timed_out"},
		{"connecting", "connection_refused"},
		{"connecting", "destination_unreachable"}
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

ClientSocket::ClientSocket
  (const ObjectCreationInfo& oi, 
	const RSocketAddress& par)
  : 
	 RSocketBase(oi, par),
	 RObjectWithEvents<ClientSocketAxis>(createdState),
	 CONSTRUCT_EVENT(connected),
	 CONSTRUCT_EVENT(connection_timed_out),
	 CONSTRUCT_EVENT(connection_refused),
	 CONSTRUCT_EVENT(destination_unreachable),

	 is_terminal_state_event {
  //is_connected_event,
		is_connection_timed_out_event,
		is_connection_refused_event,
		is_destination_unreachable_event
	 },

	 thread(dynamic_cast<Thread*>
			  (RSocketBase::repository->thread_factory
				-> create_thread(Thread::Par(this))))
{
  SCHECK(thread);
  this->RSocketBase::ancestors.push_back(this);
}

ClientSocket::~ClientSocket()
{
  RSocketBase::is_terminal_state().wait();
}

void ClientSocket::ask_connect()
{
  ::connect
	 (fd, 
	  aw_ptr->begin()->ai_addr, 
	  aw_ptr->begin()->ai_addrlen);
  process_connect_error(errno);
}

void ClientSocket::process_connect_error(int error)
{
  switch (error) {
  case EINPROGRESS:
	 State::move_to(*this, connectingState);
	 // <NB> there are no connecting->connecting transition
	 thread->start();
	 return;
  case ETIMEDOUT:
	 State::move_to(*this, connection_timed_outState);
	 return;
  case ECONNREFUSED:
	 State::move_to(*this, connection_refusedState);
	 return;
  case ENETUNREACH:
	 State::move_to(*this, destination_unreachableState);
	 return;
  case 0:
	 State::move_to(*this, connectedState);
	 return;
  default:
	 THROW_EXCEPTION(RSocketError, error);
  }
}

void ClientSocket::Thread::run()
{
  ThreadState::move_to(*this, workingState);

  fd_set wfds;
  FD_ZERO(&wfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  // Wait for connection termination
  FD_SET(fd, &wfds);
  rSocketCheck(
	 ::select(fd+1, NULL, &wfds, NULL, NULL) > 0);

  int connect_error = 0;
  socklen_t connect_error_len = sizeof(connect_error);
  rSocketCheck(
	 getsockopt(fd, SOL_SOCKET, SO_ERROR, &connect_error,
					&connect_error_len) == 0);

  dynamic_cast<ClientSocket*>(socket)
	 -> process_connect_error(connect_error);
}
