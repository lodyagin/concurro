#pragma once

#include "RSocket.h"
#include <winsock2.h>

class RSingleSocket : public RSocket
{
public:
  const bool eventUsed;

  ~RSingleSocket ();

  SOCKET get_socket () const
  {
    return socket;
  }

  WSAEVENT get_event_object ();

  DWORD get_events (bool reset_event_object = false);

  bool wait_fd_write () const 
  { return waitFdWrite; }

protected:
  // Take existing SOCKET object
  RSingleSocket (SOCKET s, bool _withEvent); 

  void init ();

  SOCKET socket;
  
  WSAEVENT socketEvent;
  // FD_WRITE is generated in 2 cases:
  // 1) after connect
  // 2) last send returns WSAWOULDBLOCK but now we can
  // send a new data
  bool waitFdWrite; 

  // Overrides
  void set_blocking (bool blocking);

};
