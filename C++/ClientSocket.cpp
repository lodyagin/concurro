// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "ClientSocket.h"
#include "Event.h"
#include "REvent.hpp"
#include "RState.hpp"

DEFINE_AXIS(
  ClientSocketAxis,
  {  "pre_connecting", // ask_connect
      "connecting"
      },
  { 
    {"created", "closed"},
    {"created", "pre_connecting"},
    {"pre_connecting", "connecting"},
    {"connecting", "ready"},
    {"connecting", "connection_timed_out"},
    {"connecting", "connection_refused"},
    {"connecting", "destination_unreachable"},
    {"ready", "closed"}
  }
  );

DEFINE_STATES(ClientSocketAxis);

DEFINE_STATE_CONST(ClientSocket, State, created);
DEFINE_STATE_CONST(ClientSocket, State, pre_connecting);
DEFINE_STATE_CONST(ClientSocket, State, connecting);
DEFINE_STATE_CONST(ClientSocket, State, ready);
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
   RStateSplitter<ClientSocketAxis, SocketBaseAxis>
     (this, createdState,
      RStateSplitter<ClientSocketAxis, SocketBaseAxis>
      ::state_hook(&ClientSocket::state_hook)
     ),
   CONSTRUCT_EVENT(pre_connecting),
   CONSTRUCT_EVENT(connecting),
   CONSTRUCT_EVENT(ready),
   CONSTRUCT_EVENT(connection_timed_out),
   CONSTRUCT_EVENT(connection_refused),
   CONSTRUCT_EVENT(destination_unreachable),
   CONSTRUCT_EVENT(closed),

   thread(dynamic_cast<Thread*>
          (RSocketBase::repository->thread_factory
           -> create_thread(Thread::Par(this))))
{
   SCHECK(thread);
   RStateSplitter<ClientSocketAxis, SocketBaseAxis>::init();
   /*this->RSocketBase::ancestor_terminals.push_back
     (is_terminal_state());*/
   this->RSocketBase::threads_terminals.push_back
      (thread->is_terminal_state());
}

ClientSocket::~ClientSocket()
{
   LOG_DEBUG(log, "~ClientSocket()");
}

void ClientSocket::ask_connect()
{
   State::move_to(*this, pre_connectingState);
   thread->start();
}

void ClientSocket::state_hook
  (AbstractObjectWithStates* object,
   const StateAxis& ax,
   const UniversalState& new_state)
{
  if (!ClientSocketAxis::is_same(ax)) {
    State::move_to(*this, 
                   RState<ClientSocketAxis>(new_state));
  }
}

void ClientSocket::process_error(int error)
{
   switch (error) {
   case EINPROGRESS:
      State::move_to(*this, connectingState);
      // <NB> there are no connecting->connecting
      // transition
      return;
   case 0:
      RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
         (*this, readyState);
      return;
   case ETIMEDOUT:
      RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
         (*this, connection_timed_outState);
      break;
   case ECONNREFUSED:
      RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
         (*this, connection_refusedState);
      break;
   case ENETUNREACH:
      RMixedAxis<ClientSocketAxis, SocketBaseAxis>::move_to
         (*this, destination_unreachableState);
      break;
   }
   //RSocketBase::process_error(error);
}

void ClientSocket::Thread::run()
{
   ThreadState::move_to(*this, workingState);
   socket->is_construction_complete_event.wait();

   auto* cli_sock = dynamic_cast<ClientSocket*>
      (socket);
   SCHECK(cli_sock);

   ( cli_sock->is_pre_connecting()
     | cli_sock->is_terminal_state()) . wait();

   if (cli_sock->is_terminal_state().signalled())
      return;

   ::connect
      (cli_sock->fd, 
       cli_sock->aw_ptr->begin()->ai_addr, 
       cli_sock->aw_ptr->begin()->ai_addrlen);
   cli_sock->process_error(errno);

   fd_set wfds;
   FD_ZERO(&wfds);

   const SOCKET fd = socket->fd;
   SCHECK(fd >= 0);

   // Wait for termination of a connection process
   FD_SET(fd, &wfds);
   rSocketCheck(
      ::select(fd+1, NULL, &wfds, NULL, NULL) > 0);
   LOG_DEBUG(ClientSocket::log, "ClientSocket>\t ::select");

   int error = 0;
   socklen_t error_len = sizeof(error);
   rSocketCheck(
      getsockopt(fd, SOL_SOCKET, SO_ERROR, &error,
                 &error_len) == 0);

   cli_sock->process_error(error);
}


