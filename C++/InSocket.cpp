// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "InSocket.h"
#include "RState.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <algorithm>

InSocket::InSocket
  (const ObjectCreationInfo& oi, 
   const RSocketAddress& par)
: 
    RSocketBase(oi, par),
    select_thread(
      dynamic_cast<SelectThread*>
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
  //add_delegate(this);
  /*this->RSocketBase::ancestor_terminals.push_back
    (is_terminal_state());*/
  this->RSocketBase::threads_terminals.push_back
    (select_thread->is_terminal_state());
  this->RSocketBase::threads_terminals.push_back
    (wait_thread->is_terminal_state());
  
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
  socket->is_construction_complete_event.wait();

  ( socket->is_ready()
    | socket->is_terminal_state()
    ) . wait();

  auto* in_sock = dynamic_cast<InSocket*>
    (socket);
  SCHECK(in_sock);

  if (socket->is_terminal_state().signalled())
    return;

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
        const int err = errno;
        LOG_ERROR(log, "Error " << rErrorMsg(err));
        break;
      }

      SCHECK( red < in_sock->socket_rd_buf_size); 
      // to make sure we always read all (rd_buf_size =
      // internal socket rcv buffer + 1)

      if (red > 0) {
        in_sock->msg.resize(red);
        //InSocket::State::move_to(*in_sock, new_dataState);

        // <NB> do not read more data until a client read
        // this piece
        in_sock->msg.is_discharged().wait();
        //InSocket::State::move_to(*in_sock, emptyState);
      }
      else {
        in_sock->msg.resize(1); //FIXME!
        in_sock->msg.resize(0);
        if (socket->is_terminal_state().signalled()) {
          break;
        }
        else {
          // peek other thread data, allow switch to it
#if 0
          std::this_thread::yield();
#else
          std::this_thread::sleep_for
            (std::chrono::milliseconds(1000));
#endif
        }
      }
    }

    //assert(InSocket::State::state_is
    //		  (*in_sock, emptyState));

    // <NB> wait for buffer discharging
    if (FD_ISSET(sock_pair[ForSelect], &rfds)) {
      // TODO actual state - closed/error (is error
      // needed?) 
      //InSocket::State::move_to (*in_sock, closedState);
      break;
    }
  }
}


void InSocket::WaitThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  socket->is_terminal_state().wait();
  static char dummy_buf[1] = {1};
  rSocketCheck(::write(notify_fd, &dummy_buf, 1) == 1);
}

