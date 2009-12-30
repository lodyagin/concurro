#pragma once
#include "rsocket.h"
#include <string>

class RInOutSocket : public RSocket
{
public:
  // Send a string message
  void send (const std::string& str);
protected:
  RInOutSocket (SOCKET s) : RSocket (s) {}
};
