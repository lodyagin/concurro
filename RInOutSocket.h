#pragma once
#include "rsocket.h"
#include <string>

class RInOutSocket : public RSocket
{
public:
  int send (void* data, int len, int* error);

  // ensure all of data on socket comes through
  size_t atomicio_send (void* data, size_t n);

protected:
  RInOutSocket (SOCKET s, bool withEvent) 
    : RSocket (s, withEvent) 
  {}
private:
  //char buf[16384];
};
