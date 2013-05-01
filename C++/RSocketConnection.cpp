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

DEFINE_STATES(
  ClientConnectionAxis,
  { "skipping", // skiping data and closing buffers
	 "aborted"   // after skipping
  },
  { { "connected", "skipping" },
	 { "skipping", "aborted" }
  }
);


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
	 socket_rep(dynamic_cast<RConnectionRepository*>
					(oi.repository)->sock_rep)
{
  assert(socket_rep);
}


RSingleSocketConnection::RSingleSocketConnection
  (const ObjectCreationInfo& oi,
   const Par& par)
 : RSocketConnection(oi, par),
	RStatesDelegator
	 (dynamic_cast<ClientSocket*>(par.socket), 
	  ClientSocket::createdState),
   socket(dynamic_cast<InSocket*>(par.socket)),
	cli_sock(dynamic_cast<ClientSocket*>(par.socket)),
	thread(dynamic_cast<SocketThread*>
			 (RThreadRepository<RThread<std::thread>>
			  ::instance().create_thread
			  (*par.get_thread_par(this)))),
	in_win(win_rep.create_object
			 (*par.get_window_par(cli_sock))),

	is_created_event(cli_sock, "created"),
	is_connected_event(cli_sock, "connected"),
	is_connection_timed_out_event
	  (cli_sock, "connection_timed_out"),
	is_connection_refused_event
	  (cli_sock, "connection_refused"),
	is_destination_unreachable_event
	  (cli_sock, "destination_unreachable"),
	is_closed_event(cli_sock, "closed"),
	is_skipping_event(this, "skipping"),
	is_aborted_event(this, "aborted"),

	is_can_abort_event { 
    is_created_event,
	 is_connected_event, 
	 is_connection_timed_out_event,
	 is_connection_refused_event, 
	 is_destination_unreachable_event,
    is_closed_event
	}
{
  assert(socket);
  assert(cli_sock);
  SCHECK(thread);
  SCHECK(in_win);
  thread->start();
}

RSingleSocketConnection::~RSingleSocketConnection()
{
  //TODO not only TCP
  //dynamic_cast<TCPSocket*>(socket)->ask_close();
  socket->ask_close_out();
  socket_rep->delete_object(socket, true);
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
		| is_skipping() ). wait();

	 if (is_skipping().signalled()) 
		goto LSkipping;

	 iw().buf.reset(new RSingleBuffer
						(std::move(socket->msg)));
	 // content of the buffer will be cleared after
	 // everybody stops using it
	 iw().buf->set_autoclear(true);
	 iw().top = 0;

	 do {
		( iw().is_filling()
		| is_skipping()) . wait();

		if (is_skipping().signalled()) 
		  goto LSkipping;

		iw().move_forward();
		STATE_OBJ(RConnectedWindow, move_to, iw(), filled);

	 } while (iw().top < iw().buf->size());

	 ( iw().is_filling()
		| is_skipping()) . wait();
		if (is_skipping().signalled()) 
		  goto LSkipping;
	 iw().buf.reset();

	 if (socket->InSocket::is_terminal_state()
		  . isSignalled())
		break;
  }

LSkipping:
  // FIXME normal close
  is_skipping().wait();
  socket->InSocket::is_terminal_state().wait();
  // No sence to start skipping while a socket is working

  if (iw().buf) {
	 assert(iw().buf->get_autoclear());
	 iw().buf.reset();
  }
  if (!STATE_OBJ(RBuffer, state_is, socket->msg, 
					  discharged))
	 socket->msg.clear();

  STATE(RSingleSocketConnection, move_to, aborted);
}

void RSingleSocketConnection::abort()
{
  // we block because no connecting -> skipping transition
  // (need change ClientSocket to allow it)
  is_can_abort().wait();

  if (RMixedAxis<ClientConnectionAxis, ClientSocketAxis>
		::compare_and_move
		(*this, ClientSocket::connectedState,
		 RSingleSocketConnection::skippingState))
	 is_aborted().wait();
}
