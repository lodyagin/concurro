// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RSocket.h"

// RSocketBase ===========================================

/*RSocketBase::RSocketBase(const ObjectCreationInfo& oi,
								 const Par& par)
{
  
}*/

 //DEFINE_STATES(InSocket, InSocketStateAxis, State)
RAxis<InSocketStateAxis> in_socket_state_axis
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


//DEFINE_STATES(OutSocket, OutSocketStateAxis, State)
RAxis<OutSocketStateAxis> out_socket_state_axis
({
  {   "wait_you",  // write buf watermark or an error
		"busy",
		"closed"      },
  { {"wait_you", "busy"},
	 {"busy", "wait_you"},
	 {"wait_you", "closed"},
	 {"busy", "closed"} }
});

DEFINE_STATE_CONST(OutSocket, State, wait_you);
DEFINE_STATE_CONST(OutSocket, State, busy);
DEFINE_STATE_CONST(OutSocket, State, closed);

InSocket::InSocket
(const ObjectCreationInfo& oi, const Par& p)
  : RObjectWithEvents<InSocketStateAxis>(emptyState),
	 msg(0)
{
  socklen_t m = sizeof(socket_rd_buf_size);
  getsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
				 &socket_rd_buf_size, &m);
  socket_rd_buf_size++; // to allow catch an overflow error
  LOG_DEBUG(log, "socket_rd_buf_size = " << socket_rd_buf_size);
}

InSocket::~InSocket()
{
  delete msg;
}

void InSocket::Thread::run()
{
  fd_set rfds;
  FD_ZERO(&rfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  REvent<DataBufferStateAxis> buf_discharged("discharged");

  for(;;) {
	 // Wait for new data
	 FD_SET(fd, &rfds);
	 rSocketCheck(
		::select(fd+1, &rfds, NULL, NULL, NULL) > 0);

	 InSocket::State::move_to(*socket, new_dataState);

	 ssize_t red;
	 rSocketCheck(
		(red = ::read(fd, socket->msg->data(),
						  socket->msg->capacity())) >= 0
		);
	 socket->msg->resize(red);

	 SCHECK((size_t) red < socket->socket_rd_buf_size); 
    // to make sure we always read all (rd_buf_size =
    // internal socket rcv buffer + 1)

	 if (red == 0) { // EOF
		InSocket::State::move_to(*socket, closedState);
		break;
	 }

	 // <NB> do not read more data until a client read this
	 // piece 
	 buf_discharged.wait(*socket->msg);
	 InSocket::State::move_to(*socket, emptyState);
  }
}


