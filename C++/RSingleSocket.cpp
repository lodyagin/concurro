// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#include "StdAfx.h"
#include "RSingleSocket.h"
#ifndef _WIN32
#  include <sys/socket.h>
#  include <fcntl.h>
#  define SD_BOTH SHUT_RDWR
#  define closesocket close
#  include <sys/select.h>
//#  include "RListeningSocket.h"
#endif

RSingleSocket::SocketEventRepository 
RSingleSocket::eventRepo("SocketEventRepository", 50);

RSingleSocket::RSingleSocket () 
  : socket (INVALID_SOCKET), 
	 eventUsed (false), 
	 waitFdWrite (false)
{
  init ();
}

RSingleSocket::RSingleSocket (SOCKET s, bool _withEvent) 
  : socket (s), eventUsed (_withEvent), waitFdWrite (false)
{
  init (); // TODO check this condition
}

RSingleSocket::~RSingleSocket ()
{
#ifndef NO_SOCKET_EVENTS
  if (eventUsed) {
#ifdef _WIN32
    ::WSACloseEvent (socketEvent); // TODO check ret
#else
	 eventRepo.delete_object(socketEvent, true);
#endif
  }
#endif

  if (socket)
  {
    ::shutdown (socket, SD_BOTH);
    ::closesocket (socket);
  }
}

void RSingleSocket::init () 
{
#ifndef NO_SOCKET_EVENTS
  if (eventUsed)
  {
#ifdef _WIN32
    const long networkEvents = 
		FD_READ | FD_WRITE | FD_CLOSE;

    socketEvent = ::WSACreateEvent ();
    if (socketEvent == WSA_INVALID_EVENT)
      THROW_EXCEPTION 
        (SException, L"WSACreateEvent call failed");

    // Start recording of socket events
    sSocketCheckWithMsg 
     (::WSAEventSelect 
        (socket, socketEvent, networkEvents)
      != SOCKET_ERROR, L" on WSAEventSelect");
#else
	 SocketEvent::Par par;
	 par.socket = this;
	 socketEvent = eventRepo.create_object(par);
	 socketEvent->start();
#endif
  }
#endif
}

void RSingleSocket::set_blocking (bool blocking)
{
#ifdef _WIN32
  u_long mode = (blocking) ? 0 : 1;
  ioctlsocket(socket, FIONBIO, &mode);
#else
  int opts = 0;
  rSocketCheck((opts = fcntl(socket, F_GETFL)) != -1);
  if (blocking)
	 opts &= (~O_NONBLOCK);
  else
	 opts |= O_NONBLOCK;
  rSocketCheck(fcntl(socket, F_SETFL, opts) != -1);
#endif
}

bool RSingleSocket::get_blocking () const
{
  int opts = 0;
  rSocketCheck(opts = fcntl(socket, F_GETFL));
  return !(opts & O_NONBLOCK);
}

#ifndef NO_SOCKET_EVENTS
#ifdef _WIN32
WSAEVENT RSingleSocket::get_event_object ()
#else
REvent* RSingleSocket::get_event_object ()
#endif
{
  if (!eventUsed)
    THROW_EXCEPTION 
      (SException, " socket event was not created");

  return socketEvent;
}

#ifdef _WIN32
DWORD RSingleSocket::get_events (bool reset_event_object)
{
  WSANETWORKEVENTS socketEvents = {0};

  sSocketCheck
    (::WSAEnumNetworkEvents 
        (socket, 
        (reset_event_object) ? socketEvent : 0, 
         &socketEvents)
     == 0);

  // check errors
  if (socketEvents.lNetworkEvents & FD_READ)
  {
    const int err = socketEvents.iErrorCode[FD_READ_BIT];
    if (err != 0) sWinErrorCode (err, L"On socket read");
  }

  if (socketEvents.lNetworkEvents & FD_WRITE)
  {
    const int err = socketEvents.iErrorCode[FD_WRITE_BIT];
    if (err != 0) sWinErrorCode (err, L"On socket write");
    waitFdWrite = false;
  }

  if (socketEvents.lNetworkEvents & FD_CLOSE)
  {
    const int err = socketEvents.iErrorCode[FD_CLOSE_BIT];
    if (err != 0) sWinErrorCode (err, L"On socket close");   
  }
  return socketEvents.lNetworkEvents;
}
#else
int32_t RSingleSocket::SocketEvent::get_events 
  (bool reset_event_object)
{
  int32_t events_val = 0;
  events.exchange(events_val);
  // use exchange to prevent a race condition with run()

  if (reset_event_object) 
	 try {
		Parent::reset();
	 }
	 catch(...) 
	 { 
		THROW_PROGRAM_ERROR; // break the atomicity
	 }

  return events_val;
}
#endif
#endif

// TODO make one high-priority thread to serve all sockets
void RSingleSocket::SocketEvent::run()
{
  // assert to ignore c) event in rfds
  // FIXME SCHECK(dynamic_cast<RListeningSocket*>(socket) == 0);

  fd_set rfds, wfds, efds;
  FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&efds);

  const int fd = socket->socket;
  
  for(;;) {
	 FD_SET(fd, &rfds);
	 FD_SET(fd, &wfds); 
	 FD_SET(fd, &efds);

	 rSocketCheck(
      //wait forewer
		::select(fd+1, &rfds, &wfds, &efds, NULL) > 0
	 );

	 const int32_t events_before = events;
	 // <NB> the events member is cleared only in
	 // get_events. 
		
	 int32_t new_events = 0;

	 if (FD_ISSET(fd, &rfds)) {
		// a) some bytes were arrived -> FD_READ
		// b) FIN was received (TCP) -> FD_READ*
		// c) accept() -> FD_READ*
		// d) a socket error is pending -> FD_READ*
		new_events |= FD_READ;
	 }

	 if (FD_ISSET(fd, &wfds)) {
		// a) there are buffer space to write bytes  
		// -> FD_WRITE

		// b) the write half of the connection is closed
		// -> FD_WRITE*
		// c) connect() completion on the socket->FD_WRITE
		// d) a socket error is pending -> FD_WRITE*
		new_events |= FD_WRITE;
	 }

	 if (FD_ISSET(fd, &efds)) {
		// out-of-band data -> FD_OOB
		new_events |= FD_OOB;
	 }

	 // Signal only new events to be compatible with
	 // Windows.
	 if ((~events_before) & new_events)
	 {
		// <NB> bits in events are set only here
		events |= new_events; 
		Parent::set();
	 }
  }
}

RThreadBase* RSingleSocket::SocketEvent::Par
::create_derivation(const ObjectCreationInfo& oi) const
{
  return new SocketEvent(oi, *this);
}
