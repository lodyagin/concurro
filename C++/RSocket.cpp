// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RSocket.h"

DEFINE_STATES(InSocket, State)
({
  {   "new_data",  // new data or an error
		"empty",
		"closed"      },
  { {"new_data", "empty"},
	 {"empty", "new_data"},
	 {"new_data", "closed"},
	 {"empty", "closed"} }
});

DEFINE_STATE_CONST(InSocket, State, new_data);
DEFINE_STATE_CONST(InSocket, State, empty);
DEFINE_STATE_CONST(InSocket, State, closed);


DEFINE_STATES(OutSocket, State)
({
  {   "wait_you",  // write buf watermark or an error
		"busy",
		"closed"      },
  { {"wait_you", "busy"},
	 {"busy", "wait_you"},
	 {"wait_you", "closed"},
	 {"busy", "closed"} }
});

DEFINE_STATE_CONST(OutSocket, State, new_data);
DEFINE_STATE_CONST(OutSocket, State, busy);
DEFINE_STATE_CONST(OutSocket, State, closed);

InSocket::InSocket
(const ObjectCreationInfo& oi, const Par& p)
{
  socklen_t m = sizeof(socket_rd_buf_size);
  getsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
				 &socket_rd_buf_size, &m);
  rd_buf_size++; // to allow catch an overflow error
  LOG_DEBUG(log, "socket_rd_buf_size = " << rd_buf_size);
}

void InSocket::Thread::run()
{
  fd_set rfds;
  FD_ZERO(&rfds);

  const int fd = socket->socket;
  SCHECK(fd >= 0);

  for(;;) {
	 if (socket->state_is(socket->closedState))
		return;

	 InSocket::State::check_moving_to
		(*socket, new_dataState);

	 // Wait for new data
	 FD_SET(fr, &rfds);
	 rSocketCheck(
		::select(fd+1, &rfds, NULL, NULL, NULL) > 0);

	 InSocket::State::move_to(*socket, new_dataState);

	 ssize_t red;
	 rSocketCheck(
		(red = ::read(fd, msg.data(), msg.reserved())));
	 msg.size(red);

	 SCHECK(red < socket_rd_buf_size); 
    // to make sure we always read all (rd_buf_size =
    // internal socket rcv buffer + 1)

	 msg.discharged.wait();
  }
}


