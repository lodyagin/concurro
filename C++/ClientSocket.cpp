// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "ClientSocket.h"

DEFINE_STATES(ClientSocket, ClientSocketStateAxis, State)
(  {"created",
    "connecting",  
	 "connected",
	 "connection_timed_out",
	 "connection_refused",
	 "destination_unreachable",
	 "destroyed" // to check a state in the destructor
	 },
    { 
		{"created", "connecting"},
		{"connecting", "connected"},
		{"connecting", "connection_timed_out"},
		{"connecting", "connection_refused"},
		{"connecting", "destination_unreachable"},
		{"connection_timed_out", "destroyed"},
		{"connection_refused", "destroyed"},
		{"destination_unreachable", "destroyed"}
	 }
  });
DEFINE_STATE_CONST(ClientSocket, State, created);
DEFINE_STATE_CONST(ClientSocket, State, connecting);
DEFINE_STATE_CONST(ClientSocket, State, connected);
DEFINE_STATE_CONST(ClientSocket, State, 
						 connection_timed_out);
DEFINE_STATE_CONST(ClientSocket, State, 
						 connection_refused);
DEFINE_STATE_CONST(ClientSocket, State, 
						 destination_unreachable);
DEFINE_STATE_CONST(ClientSocket, State, destroyed);


ClientSocket::ClientSocket() 
  : RObjectWithEvents<ClientSocketAxis> (createdState),
	 thread(
{
  ask_connect();
}

ClientSocket::~ClientSocket()
{
  State::move_to(*this, destroyedState);
}

void ClientSocket::ask_connect()
{
  const int res = ::connect
	 (fd, aw_ptr->ai_addr, aw_ptr->ai_addrlen);
  process_connect_error(res);
}

void process_connect_error(int error)
{
  switch (error) {
  case EINPROGRESS:
	 State::move_to(*this, connectingState);
	 // <NB> there are no connecting->connecting transition
	 thread.start();
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
  fd_set wfds;
  FD_ZERO(&wfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  // Wait for connection termination
  FD_SET(fd, &wfds);
  rSocketCheck(
	 ::select(fd+1, NULL, &wfds, NULL, NULL) > 0);

  int connect_error = 0;
  rSocketCheck(
	 getsockopt(fd, SOL_SOCKET, SO_ERROR, &connect_error,
					sizeof(connect_error)) == 0);

  process_connect_error(connect_error);
}
