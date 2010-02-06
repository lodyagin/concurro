#include "StdAfx.h"
#include "RSingleSocket.h"

RSingleSocket::RSingleSocket ()
 : socket (0), eventUsed (false),
  waitFdWrite (false)
{
}

RSingleSocket::RSingleSocket (SOCKET s, bool _withEvent) 
  : socket (s), eventUsed (_withEvent), waitFdWrite (false)
{
  if (eventUsed)
  {
    const long networkEvents = FD_READ | FD_WRITE;

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
}

RSingleSocket::~RSingleSocket ()
{
  if (eventUsed)
    ::WSACloseEvent (socketEvent); // TODO check ret

  if (socket)
  {
    ::shutdown (socket, SD_BOTH);
    ::closesocket (socket);
  }
}

void RSingleSocket::set_blocking (bool blocking)
{
  u_long mode = (blocking) ? 0 : 1;
  ioctlsocket(socket, FIONBIO, &mode);
}

WSAEVENT RSingleSocket::get_event_object ()
{
  if (!eventUsed)
    THROW_EXCEPTION 
      (SException, L" socket event was not created");

  return socketEvent;
}

DWORD RSingleSocket::get_events ()
{
  WSANETWORKEVENTS socketEvents = {0};

  sSocketCheck
    (::WSAEnumNetworkEvents 
        (socket, 0, &socketEvents)
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
  return socketEvents.lNetworkEvents;
}
