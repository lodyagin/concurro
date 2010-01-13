#pragma once
#include "SNotCopyable.h"
#include <winsock2.h>

class RSocket : public SNotCopyable
{
public:
  virtual ~RSocket ();

  SOCKET get_socket () const
  {
    return socket;
  }

protected:
  SOCKET socket;
  
  RSocket () : socket (0) {}
  RSocket (SOCKET s) : socket (s) {}

  // set blocking mode
  void set_blocking (bool blocking);
};
