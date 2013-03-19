#include "StdAfx.h"
#include "RSingleSocket.h"
#ifndef _WIN32
#  include <sys/socket.h>
#  include <fcntl.h>
#  define SD_BOTH SHUT_RDWR
#  define closesocket close
#endif

RSingleSocket::RSingleSocket () 
  : socket (INVALID_SOCKET), eventUsed (false), waitFdWrite (false)
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
  if (eventUsed)
    ::WSACloseEvent (socketEvent); // TODO check ret
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
    const long networkEvents = FD_READ | FD_WRITE | FD_CLOSE;

    socketEvent = ::WSACreateEvent ();
    if (socketEvent == WSA_INVALID_EVENT)
      THROW_EXCEPTION 
        (SException, L"WSACreateEvent call failed");

    // Start recording of socket events
    sSocketCheckWithMsg 
     (::WSAEventSelect 
        (socket, socketEvent, networkEvents)
      != SOCKET_ERROR, L" on WSAEventSelect");
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
WSAEVENT RSingleSocket::get_event_object ()
{
  if (!eventUsed)
    THROW_EXCEPTION 
      (SException, L" socket event was not created");

  return socketEvent;
}

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
#endif
