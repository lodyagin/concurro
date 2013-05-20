// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocketConnection.h"
#include "OutSocket.h"
#include "ClientSocket.h"
#include "REvent.hpp"
#include "RState.hpp"

DEFINE_AXIS(
  ClientConnectionAxis,
  { "aborting", // skiping data and closing buffers
     "aborted",   // after aborting
     "clearly_closed" // all pending data 
                      // was received / sent
  },
  { { "ready", "aborting" },
    { "aborting", "aborted" },
    { "closed", "clearly_closed" },
    { "closed", "aborting" }
  }
);

DEFINE_STATES(ClientConnectionAxis);

DEFINE_STATE_CONST(RSingleSocketConnection, State, 
                   aborting);
DEFINE_STATE_CONST(RSingleSocketConnection, State, 
                   aborted);
DEFINE_STATE_CONST(RSingleSocketConnection, State, 
                   clearly_closed);

std::ostream&
operator<< (std::ostream& out, const RSocketConnection& c)
{
  out << "<connection>";
  return out;
}

RSocketConnection::RSocketConnection
(const ObjectCreationInfo& oi,
 const Par& par)
  : StdIdMember(oi.objectId),
    win_rep("RSocketConnection::win_rep", 
            par.win_rep_capacity),
    socket_rep(std::move(par.socket_rep))
{
  assert(socket_rep);
}

RSingleSocketConnection::RSingleSocketConnection
(const ObjectCreationInfo& oi,
 const Par& par)
  : RSocketConnection(oi, par),
    RStateSplitter
    (dynamic_cast<ClientSocket*>(par.socket), 
     ClientSocket::createdState),
    CONSTRUCT_EVENT(aborting),
    CONSTRUCT_EVENT(aborted),
    CONSTRUCT_EVENT(clearly_closed),
    socket(dynamic_cast<InSocket*>(par.socket)),
    cli_sock(dynamic_cast<ClientSocket*>(par.socket)),
    thread(dynamic_cast<SocketThread*>
           (RThreadRepository<RThread<std::thread>>
            ::instance().create_thread
            (*par.get_thread_par(this)))),
    in_win(win_rep.create_object
           (*par.get_window_par(cli_sock))),
    is_closed_event(cli_sock, "closed"),
    is_terminal_state_event { 
      is_clearly_closed_event,
      is_aborted_event
    }
{
  assert(socket);
  assert(cli_sock);
  SCHECK(thread);
  SCHECK(in_win);
  RStateSplitter::init();
  thread->start();
}

RSingleSocketConnection::~RSingleSocketConnection()
{
  //TODO not only TCP
  //dynamic_cast<TCPSocket*>(socket)->ask_close();
  //socket->ask_close_out();
  is_terminal_state_event.wait();
  socket_rep->delete_object(socket, true);
}

void RSingleSocketConnection::state_changed
  (StateAxis& ax, 
   const StateAxis& state_ax,     
   AbstractObjectWithStates* object)
{
  RState<ClientConnectionAxis> st =
    state_ax.bound(object->current_state(state_ax));

  // aborting state check
  if (st == RState<ClientConnectionAxis>
      (ClientSocket::closedState)
      && RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
      ::compare_and_move
      (*this, abortingState, abortedState))
    return;

  // aborted state check
  if (st == RState<ClientConnectionAxis>
      (ClientSocket::closedState)
      && A_STATE(RSingleSocketConnection, 
                 ClientConnectionAxis, state_is, aborted))
    return;

  RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
    ::neg_compare_and_move(*this, st, st);

  LOG_TRACE(log, "moved to " << st);
}

RSocketConnection& RSingleSocketConnection
::operator<< (const std::string str)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  out_sock->msg.is_discharged().wait();
  out_sock->msg.reserve(str.size());
  ::strncpy((char*)out_sock->msg.data(), str.c_str(),
            str.size());
  out_sock->msg.resize(str.size());
  return *this;
}

RSocketConnection& RSingleSocketConnection
::operator<< (RSingleBuffer&& buf)
{
  auto* out_sock = dynamic_cast<OutSocket*>(socket);
  SCHECK(out_sock);

  out_sock->msg = std::move(buf);
  return *this;
}

void RSingleSocketConnection::ask_connect()
{
  dynamic_cast<ClientSocket*>(socket)->ask_connect();
}

void RSingleSocketConnection::ask_close()
{
  socket->ask_close_out();
}

void RSingleSocketConnection::run()
{
  socket->is_construction_complete_event.wait();

  for (;;) {
    ( socket->msg.is_charged()
      | is_aborting() ). wait();

    if (is_aborting().signalled()) 
      goto LAborting;

    iw().buf.reset(new RSingleBuffer
                   (std::move(socket->msg)));
    // content of the buffer will be cleared after
    // everybody stops using it
    iw().buf->set_autoclear(true);
    iw().top = 0;

    CompoundEvent ce1 {
      iw().is_filling(), is_aborting(), is_terminal_state()
    };

    do {
      // wait a new data request 
      // (e.g. RWindow::forward_top(size))
      ce1.wait();

      if (is_aborting().signalled()) 
        goto LAborting;
      else if (is_terminal_state().signalled()) 
        goto LClosed;

      iw().move_forward(); // filling the window
      STATE_OBJ(RConnectedWindow, move_to, iw(), filled);

    } while (iw().top < iw().buf->size());

    // All the buffer is red
    // Allocate a new buffer only on a next data request.
    ce1.wait();

    if (is_aborting().signalled()) 
      goto LAborting;
    else if (is_terminal_state().signalled()) 
      goto LClosed;

    iw().buf.reset();

    //if (is_terminal_state().signalled())
    //  break;
  }

LAborting:
  is_aborting().wait();
  ask_close();
  socket->InSocket::is_terminal_state().wait();
  // No sence to start aborting while a socket is working

  assert(iw().buf->get_autoclear());
  RWindow(iw()); // clear iw()
#if 0
  if (iw().buf) {
    assert(iw().buf->get_autoclear());
    iw().buf.reset(); // reset shared_ptr
  }
#endif
  if (!STATE_OBJ(RBuffer, state_is, socket->msg, 
                 discharged))
    socket->msg.clear();

LClosed: ;
}

void RSingleSocketConnection::ask_abort()
{
#if 1
  A_STATE(RSingleSocketConnection, 
          ClientConnectionAxis, move_to, aborting);
#else
  // we block because no connecting -> aborting transition
  // (need change ClientSocket to allow it)
  is_can_abort().wait();

  if (RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
      ::compare_and_move
      (*this, ClientSocket::connectedState,
       RSingleSocketConnection::abortingState))
    is_aborted().wait();
#endif
}
