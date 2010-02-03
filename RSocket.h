#pragma once
#include "SNotCopyable.h"
#include <winsock2.h>

class RSocket : public SNotCopyable
{
public:
  const bool eventUsed;

  virtual ~RSocket ();

  SOCKET get_socket () const
  {
    return socket;
  }

  WSAEVENT get_event_object ();

  DWORD get_events ();

  bool wait_fd_write () const { return waitFdWrite; }

protected:
  SOCKET socket;
  
  RSocket ();

  RSocket (SOCKET s, bool _withEvent); 

  // set blocking mode
  void set_blocking (bool blocking);

  WSAEVENT socketEvent;
  // FD_WRITE is generated in 2 cases:
  // 1) after connect
  // 2) last send returns WSAWOULDBLOCK but now we can
  // send a new data
  bool waitFdWrite; 
};
