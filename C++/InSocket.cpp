// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "InSocket.h"
#include <sys/types.h>
#include <sys/socket.h>

RAxis<InSocketStateAxis> in_socket_state_axis
({
  {   "new_data",  // new data or an error
      "empty",
      "closed",
		"error"
//		"destroyed"
  },
  { {"new_data", "empty"},
  {"empty", "new_data"},
  {"new_data", "closed"},
  {"empty", "closed"},
  {"empty", "error"},
  {"error", "closed"}//,
//  {"closed", "destroyed"}
  }
});
DEFINE_STATE_CONST(InSocket, State, new_data);
DEFINE_STATE_CONST(InSocket, State, empty);
DEFINE_STATE_CONST(InSocket, State, closed);


InSocket::InSocket
  (const ObjectCreationInfo& oi, 
	const RSocketAddress& par)
: 
	 RSocketBase(oi, par),
	 RObjectWithEvents<InSocketStateAxis>(emptyState),
	 thread(dynamic_cast<Thread*>
			  (RSocketBase::repository->thread_factory
				-> create_thread(Thread::Par(this))))
{
  SCHECK(thread);
  socklen_t m = sizeof(socket_rd_buf_size);
  getsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
     &socket_rd_buf_size, &m);
  socket_rd_buf_size++; //to allow catch an overflow error
  LOG_DEBUG(log, "socket_rd_buf_size = " 
               << socket_rd_buf_size);
  msg.reserve(socket_rd_buf_size);
}

InSocket::~InSocket()
{
  LOG_DEBUG(log, "~InSocket()");
}

void InSocket::ask_close()
{
}

void InSocket::Thread::run()
{
  fd_set rfds;
  FD_ZERO(&rfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  static REvent<DataBufferStateAxis> buf_discharged
	 (*sock, "discharged");

  for(;;) {
    // Wait for new data
	 // The second socket for close report
	 FD_SET(sock_pair[ForSelect], &rfds);
    //FD_SET(fd, &rfds);
	 const int maxfd = max(sock_pair[ForSelect], fd) + 1;
	 //const int maxfd = fd;
    rSocketCheck(
		 ::select(maxfd, &rfds, NULL, NULL, NULL) > 0);

	 if (FD_ISSET(fd, &rfds)) {
		const ssize_t red = ::read(fd, socket->msg.data(),
											socket->msg.capacity());
		if (red < 0) {
		  State::move_to(*sock, errorState);
		  // TODO add the error code
		  break;
		}
		/*else if (red == 0) {
		  //EOF
		  State::move_to(*sock, closedState);
		  break;
		  }*/

		SCHECK((size_t) red < socket->socket_rd_buf_size); 
		// to make sure we always read all (rd_buf_size =
		// internal socket rcv buffer + 1)

		socket->msg.resize(red);
		InSocket::State::move_to(*socket, new_dataState);

		// <NB> do not read more data until a client read
		// this piece
		buf_discharged.wait(socket->msg);
		InSocket::State::move_to(*socket, emptyState);
	 }
  }
}


