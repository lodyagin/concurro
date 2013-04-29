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
//#include "TCPSocket.h"

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
	 socket_rep(dynamic_cast<RConnectionRepository*>
					(oi.repository)->sock_rep)
{
  assert(socket_rep);
}


RSingleSocketConnection::RSingleSocketConnection
  (const ObjectCreationInfo& oi,
   const Par& par)
 : RSocketConnection(oi, par),
   socket(dynamic_cast<InSocket*>
			 (socket_rep->create_object(*par.sock_addr))),
	thread(dynamic_cast<SocketThread*>
			 (RThreadRepository<RThread<std::thread>>
			  ::instance().create_thread
			  (*par.get_thread_par(this)))),
	win(SFORMAT("RSingleSocketConnection:" << socket->fd))
{
  SCHECK(thread);
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
		| win.is_skipping() ). wait();

	 if (win.is_skipping().signalled()) 
		goto LSkipping;

	 win.buf.reset(new RSingleBuffer
						(std::move(socket->msg)));
	 // content of the buffer will be cleared after
	 // everybody stops using it
	 win.buf->set_autoclear(true);
	 win.top = 0;

	 do {
		( win.is_filling()
		| win.is_skipping()) . wait();

		if (win.is_skipping().signalled()) 
		  goto LSkipping;

		win.move_forward();

	 } while (win.top < win.buf->size());

	 ( win.is_filling()
		| win.is_skipping()) . wait();
		if (win.is_skipping().signalled()) 
		  goto LSkipping;
	 win.buf.reset();

	 if (socket->InSocket::is_terminal_state().isSignalled())
		break;
  }

LSkipping:
  win.is_skipping().wait();
  socket->InSocket::is_terminal_state().wait();
  // No sence to start skipping while a socket is working

  if (win.buf) {
	 assert(win.buf->get_autoclear());
	 win.buf.reset();
  }
  if (!STATE_OBJ(RBuffer, state_is, socket->msg, 
					  discharged))
	 socket->msg.clear();

  STATE_OBJ(RWindow, move_to, win, destroyed);
}
