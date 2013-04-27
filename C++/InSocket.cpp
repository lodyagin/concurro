// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "InSocket.h"
#include "TCPSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <algorithm>

RAxis<InSocketAxis> in_socket_state_axis
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
DEFINE_STATE_CONST(InSocket, State, error);


InSocket::InSocket
  (const ObjectCreationInfo& oi, 
	const RSocketAddress& par)
: 
	 RSocketBase(oi, par),
	 RObjectWithEvents<InSocketAxis>(emptyState),
	 CONSTRUCT_EVENT(new_data),
	 CONSTRUCT_EVENT(error),
	 CONSTRUCT_EVENT(closed),
	 select_thread(dynamic_cast<SelectThread*>
			  (RSocketBase::repository->thread_factory
				-> create_thread(SelectThread::Par(this)))),
	 wait_thread(
		dynamic_cast<WaitThread*>
		(RSocketBase::repository->thread_factory
		 -> create_thread
		 (WaitThread::Par
		  (this, 
			select_thread->get_notify_fd()))))
{
  SCHECK(select_thread && wait_thread);
  this->RSocketBase::ancestor_terminals.push_back
	 (is_terminal_state());
  this->RSocketBase::threads_terminals.push_back
	 (select_thread->is_terminated());
  this->RSocketBase::threads_terminals.push_back
	 (wait_thread->is_terminated());
  
  socklen_t m = sizeof(socket_rd_buf_size);
  rSocketCheck(
	 getsockopt(fd, SOL_SOCKET, SO_RCVBUF, 
					&socket_rd_buf_size, &m) == 0);
  socket_rd_buf_size++; //to allow catch an overflow error
  LOG_DEBUG(log, "socket_rd_buf_size = " 
               << socket_rd_buf_size);
  msg.reserve(socket_rd_buf_size);
  select_thread->start();
  wait_thread->start();
}

InSocket::~InSocket()
{
  LOG_DEBUG(log, "~InSocket()");
}

void InSocket::ask_close()
{
}

void InSocket::SelectThread::run()
{
  ThreadState::move_to(*this, workingState);

  ( socket->is_ready()
  | socket->is_closed()
  | socket->is_error()) . wait();

  auto* in_sock = dynamic_cast<InSocket*>
	 (socket);
  SCHECK(in_sock);

  if (socket->is_closed().signalled()) {
	 InSocket::State::move_to
		(*in_sock, InSocket::closedState);
	 return;
  }
  else if (socket->is_error().signalled()) {
	 InSocket::State::move_to
		(*in_sock, InSocket::errorState);
	 return;
  }

  fd_set rfds;
  FD_ZERO(&rfds);

  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  for(;;) {
    // Wait for new data
	 if (RSocketBase::State::state_is
		  (*socket, RSocketBase::readyState))
		FD_SET(fd, &rfds);
	 else
		FD_CLR(fd, &rfds);
	 // The second socket for close report
	 FD_SET(sock_pair[ForSelect], &rfds);
	 const int maxfd = std::max(sock_pair[ForSelect], fd)
		+ 1;
    rSocketCheck(
		 ::select(maxfd, &rfds, NULL, NULL, NULL) > 0);
	 LOG_DEBUG(log, "InSocket>\t ::select");

	 if (FD_ISSET(fd, &rfds)) {
		const ssize_t red = ::read(fd, in_sock->msg.data(),
											in_sock->msg.capacity());
		if (red < 0) {
		  if (errno == EAGAIN) continue;
		  in_sock->process_error(errno);
		  // TODO add the error code
		  break;
		}

		SCHECK( red < in_sock->socket_rd_buf_size); 
		// to make sure we always read all (rd_buf_size =
		// internal socket rcv buffer + 1)

		in_sock->msg.resize(red);
		if (red > 0) {
		  InSocket::State::move_to(*in_sock, new_dataState);

		  // <NB> do not read more data until a client read
		  // this piece
		  in_sock->msg.is_discharged().wait();
		  InSocket::State::move_to(*in_sock, emptyState);
		}
	 }

	 assert(InSocket::State::state_is
			  (*in_sock, emptyState));

	 // <NB> wait for buffer discharging
	 if (FD_ISSET(sock_pair[ForSelect], &rfds)) {
		// TODO actual state - closed/error (is error
		// needed?) 
		InSocket::State::move_to (*in_sock, closedState);
		break;
	 }
  }
}


void InSocket::WaitThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  (socket->is_closed() | socket->is_error()).wait();
  static char dummy_buf[1] = {1};
  rSocketCheck(::write(notify_fd, &dummy_buf, 1) == 1);
}

