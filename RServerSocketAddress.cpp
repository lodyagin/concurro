#include "stdafx.h"
#include "RServerSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Ws2bth.h>

RServerSocketAddress::RServerSocketAddress 
  (unsigned int port)
{
  struct addrinfo hints = {0};

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  const char* hostname = NULL;

  // make a string representation of a port
  std::string portStr;
  ::toString (port, portStr);

  init (hostname, portStr.c_str (), hints);

  LOG4STRM_DEBUG 
    (Logging::Root (), 
    oss_ << "New RServerSocketAddress is created: ";
    outString (oss_)
    );
}

void RServerSocketAddress::outString 
  (std::ostream& out) const
{
  out << "RServerSocketAddress:\n";
  RMultiprotoSocketAddress::outString (out);
  out << '\n';
}

