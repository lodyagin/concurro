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

  DWORD get_events ();

  bool wait_fd_write () const 
  { return waitFdWrite; }

protected:
  RSingleSocket ();

  SOCKET socket;
  
  RSingleSocket (SOCKET s, bool _withEvent); 

  WSAEVENT socketEvent;
  // FD_WRITE is generated in 2 cases:
  // 1) after connect
  // 2) last send returns WSAWOULDBLOCK but now we can
  // send a new data
  bool waitFdWrite; 

  // Overrides
  void set_blocking (bool blocking);

};
