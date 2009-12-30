#pragma once
#include "SNotCopyable.h"
#include <winsock2.h>

class RSocket : public SNotCopyable
{
public:
  virtual ~RSocket ();

protected:
  SOCKET socket;
  
  RSocket () : socket (0) {}
  RSocket (SOCKET s) : socket (s) {}

  // set blocking mode
  void set_blocking (bool blocking);
};
