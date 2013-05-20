// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "TCPSocket.h"
#include "ClientSocket.h"
#include "REvent.hpp"
#include "RState.hpp"

DEFINE_AXIS(
  TCPAxis, 
  {
    "in_closed",    // input part of connection is closed
    "out_closed",
    "listen",       // passive open
    "accepting",    // in the middle of a new ServerSocket
    "closing",      // fin-wait, time-wait, closing,
  },
  {
    {"created", "listen"},      // listen()
    {"listen", "accepting"}, // connect() from other side
    {"accepting", "listen"},
    {"listen", "closed"},
    {"created", "ready"}, // initial send() is
      // recieved by other side
    {"created", "closed"},     // ask close() or timeout 
    {"ready", "closing"}, // our close() or FIN from
      // other side
    {"ready", "in_closed"},
    {"ready", "out_closed"},
    {"closing", "closed"},
    {"in_closed", "closed"},
    {"out_closed", "closed"},
    {"closed", "closed"}
  }
  );

DEFINE_STATES(TCPAxis);

DEFINE_STATE_CONST(TCPSocket, State, created);
DEFINE_STATE_CONST(TCPSocket, State, closed);
DEFINE_STATE_CONST(TCPSocket, State, in_closed);
DEFINE_STATE_CONST(TCPSocket, State, out_closed);
DEFINE_STATE_CONST(TCPSocket, State, listen);
DEFINE_STATE_CONST(TCPSocket, State, accepting);
DEFINE_STATE_CONST(TCPSocket, State, ready);
DEFINE_STATE_CONST(TCPSocket, State, closing);

TCPSocket::TCPSocket
(const ObjectCreationInfo& oi, 
 const RSocketAddress& par)
  : 
  RSocketBase(oi, par),
  RStateSplitter<TCPAxis, SocketBaseAxis>
    (this, createdState,
     RStateSplitter<TCPAxis, SocketBaseAxis>
     ::state_hook(&TCPSocket::state_hook)
    ),
  CONSTRUCT_EVENT(ready),
  CONSTRUCT_EVENT(closed),
  CONSTRUCT_EVENT(in_closed),
  CONSTRUCT_EVENT(out_closed),
  tcp_protoent(NULL),
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
  RStateSplitter<TCPAxis, SocketBaseAxis>::init();
  /*this->RSocketBase::ancestor_terminals.push_back
    (is_terminal_state());*/
  this->RSocketBase::threads_terminals.push_back
    (select_thread->is_terminal_state());
  this->RSocketBase::threads_terminals.push_back
    (wait_thread->is_terminal_state());
  SCHECK((tcp_protoent = ::getprotobyname("TCP")) !=
         NULL);
  //thread->start();
}

TCPSocket::~TCPSocket()
{
  LOG_DEBUG(log, "~TCPSocket()");
}

void TCPSocket::state_hook
  (AbstractObjectWithStates* object,
   const StateAxis& ax,
   const UniversalState& new_state)
{
  if (!TCPAxis::is_same(ax)) {
    const RState<TCPAxis> st(new_state);
    State::move_to(*this, st);

    if (st == readyState) {
      select_thread->start();
      wait_thread->start();
    }
  }
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
      A_STATE(TCPSocket, TCPAxis, move_to, closed);
      RSocketBase::State::compare_and_move
        (*this, 
         { RSocketBase::readyState,
             RSocketBase::createdState
             }, 
         RSocketBase::closedState);
      return;
    }
  }

#if 1
  State::compare_and_move
    (*this, readyState, out_closedState) ||
  State::compare_and_move
    (*this, in_closedState, closedState);
#else
  if (State::compare_and_move
      (*this, readyState, out_closedState)
      ||
      State::compare_and_move
      (*this, in_closedState, closedState)
    )
    RSocketBase::State::move_to
      (*this, RSocketBase::closedState);
#endif
}

void TCPSocket::SelectThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();
  //ClientSocket* cli_sock = 0;
  //if (!(cli_sock = dynamic_cast<ClientSocket*>(socket))) 
  // THROW_NOT_IMPLEMENTED;
  TCPSocket* tcp_sock = dynamic_cast<TCPSocket*>(socket);
  assert(tcp_sock);
  
  /*( socket->is_ready()
    | socket->is_terminal_state()) . wait();

    if (socket->is_terminal_state().isSignalled())
    return;*/

  // wait for close
  fd_set wfds, rfds;
  FD_ZERO(&wfds);
  FD_ZERO(&rfds);
  const SOCKET fd = socket->fd;
  SCHECK(fd >= 0);

  for (;;) {
    if (A_STATE_OBJ(TCPSocket, TCPAxis, state_is, 
                    *tcp_sock, in_closed)
        || A_STATE_OBJ(TCPSocket, TCPAxis, state_is, 
                       *tcp_sock, closed))
      FD_CLR(fd, &rfds);
    else
      FD_SET(fd, &rfds);
    FD_SET(sock_pair[ForSelect], &rfds);
    const int maxfd = std::max(sock_pair[ForSelect], fd)
      + 1;

    /*if (State::state_is(*tcp_sock, out_closedState))
      FD_CLR(fd, &wfds);
      else
      FD_SET(fd, &wfds);*/

    rSocketCheck(
      ::select(maxfd, &rfds, NULL, NULL, NULL) > 0);
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

        if (TCPSocket::State::state_is
            (*tcp_sock, TCPSocket::closedState))
          break;

        if (TCPSocket::State::compare_and_move
            (*tcp_sock, 
             TCPSocket::readyState, 
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

    if (FD_ISSET(sock_pair[ForSelect], &rfds)) {
      break;
    }

    //if (FD_ISSET(fd, &wfds)) {
    // FIXME need intercept SIGPIPE
    //}
  }
}

void TCPSocket::WaitThread::run()
{
  ThreadState::move_to(*this, workingState);
  socket->is_construction_complete_event.wait();

  socket->is_terminal_state().wait();
  static char dummy_buf[1] = {1};
  rSocketCheck(::write(notify_fd, &dummy_buf, 1) == 1);
}

