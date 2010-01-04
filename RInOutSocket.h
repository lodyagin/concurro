#pragma once
#include "rsocket.h"
#include <string>

class RInOutSocket : public RSocket
{
public:
  // Send a string message
  void send (const std::string& str);
  void receive (std::string& out);
protected:
  RInOutSocket (SOCKET s) : RSocket (s) {}
private:
  char buf[16384];
};
