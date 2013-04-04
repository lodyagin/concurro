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

RSingleSocket::RSingleSocket (bool _withEvent) 
  : socket (INVALID_SOCKET), 
	 eventUsed (_withEvent), 
	 waitFdWrite (false),
	 socketCreated(true, false),
	 rd_buf(0),
	 rd_buf_size(0)
{
  init ();
}

RSingleSocket::RSingleSocket 
  (SOCKET s, bool _withEvent) 
  : socket (s), 
	 eventUsed (_withEvent), 
	 waitFdWrite(false),
	 socketCreated(true, true)
{
  SCHECK(s != INVALID_SOCKET);
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

// We implement double-buffering to clear a socket event
// after reporting.
void RSingleSocket::SocketEvent::run()
{
  // assert to ignore c) event in rfds
  // FIXME 
  // SCHECK(dynamic_cast<RListeningSocket*>(socket) == 0);

  fd_set rfds, wfds, efds;
  FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&efds);

  // wait the actual socket->socket creation
  // socket->socketCreated.wait(); 

  const int fd = socket->socket;
  SCHECK(fd >= 0);

  // get the socket buffer size
  size_t rd_buf_size = 0;//, wr_buf_size = 0;
  {
	 socklen_t m = sizeof(rd_buf_size);
	 getsockopt(fd, SOL_SOCKET, SO_RCVBUF,
					&rd_buf_size,&m);
	 rd_buf_size++; // to allow catch an overflow error
	 LOG_DEBUG(log, "rd_buf_size = " << rd_buf_size);
/*	 getsockopt(fd, SOL_SOCKET, SO_SNDBUF,
					&wr_buf_size,&m);
	 wr_buf_size++;
	 LOG_DEBUG(log, "wr_buf_size = " << wr_buf_size);*/
  }

  // We implement double-buffering to allow clear already
  // reported socket events
//  std::auto_ptr<char*> wr_buf (new char[wr_buf_size]);

  for(;;) {
	 // invert events to skip events not red by get_events yet
	 if (events & FD_READ) FD_RESET(fd, &rfds);
	 else FD_SET(fd, &rfds);

	 if (events & FD_WRITE) FD_RESET(fd, &wfds);
	 else FD_SET(fd, &wfds);

	 if (events & FD_OOD) FD_RESET(fd, &efds);
	 else FD_SET(fd, &efds);

	 rSocketCheck(
      //wait forewer
		::select(fd+1, &rfds, &wfds, &efds, NULL) > 0
	 );

	 int32_t new_events = 0;

	 if (FD_ISSET(fd, &rfds)) {
		// a) some bytes were arrived -> FD_READ
		// b) FIN was received (TCP) -> FD_READ*
		// c) accept() -> FD_READ*
		// d) a socket error is pending -> FD_READ*
		new_events |= FD_READ;
		
		ssize_t red;
		rSocketCheck((red = ::read(fd, rd_buf,
											rd_buf_size)));
		SCHECK(red < rd_buf_size); // to make sure we always
											// read all (rd_buf_size
											// = internal socket rcv
											// buffer + 1
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

	 LOG_DEBUG(log, "New events: " << new_events);

	 // Signal only new events to be compatible with
	 // Windows.
	 if ((~events_before) & new_events)
	 {
		// <NB> bits in events are set only here
		events |= new_events; 
		LOG_DEBUG(Logger<LOG::Concurrency>,
					 "Parent::set()");
		Parent::set();
	 }
  }
}

RThreadBase* RSingleSocket::SocketEvent::Par
::create_derivation(const ObjectCreationInfo& oi) const
{
  return new SocketEvent(oi, *this);
}

int RSignleSocket::send (void* data, int len, int* error)
{
    int lenSent = ::send(socket, (const char*) data, len, 0);
		if (lenSent == -1) {
#ifdef _WIN32
      const int err = ::WSAGetLastError ();
#else
      const int err = errno;
#endif
			if (err == WSAEINTR || 
			    err == WSAEWOULDBLOCK)
      {
        if (err == WSAEWOULDBLOCK) waitFdWrite = true;
        *error = err;
				return -1;
      }
      THROW_EXCEPTION(SException,
							 SFORMAT("Write failed: " << strerror(err)));
		}
    *error = 0;
    return lenSent;
}

#if 0 // FIX sleep
size_t RSingleSocket::atomic_send (void* data, size_t n)
{
	char *s = reinterpret_cast<char*> (data);
	size_t pos = 0;
	int res, error;

	while (n > pos) {
		res = this->send (s + pos, n - pos, &error);
    if (error)
    {
	    if (error == WSAEINTR)
		    continue;

      if (error == WSAEWOULDBLOCK) {
          ::Sleep(1000); // FIXME
				  continue;
			}

      return 0;
    }

    if (res == 0)
      THROW_EXCEPTION
        (SException,
         oss_ << L"The connection is closed by peer");

  	pos += (size_t)res;
	}
	return (pos);
}
#endif
