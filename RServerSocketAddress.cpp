#include "stdafx.h"
#include "RServerSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

RServerSocketAddress::RServerSocketAddress 
  (unsigned int port)
{
  struct addrinfo hints = {0};

  hints.ai_family = AF_INET; //TODO set AF_UNSPEC
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  //hints.ai_protocol = IPPROTO_TCP;

  const char* hostname = NULL;

  // make a string representation of a port
  std::string portStr;
  ::toString (port, portStr);

  LOG4STRM_DEBUG
    (Logging::Root (),
    oss_ << "Call getaddrinfo (" 
         << ((hostname) ? hostname : NULL)
         << ", [" << portStr.c_str () << "], ";
    outString (oss_, &hints)
     );

  sSocketCheck //FIXME gai_strerror must be used for formatting an error
    (::getaddrinfo 
      (hostname, portStr.c_str (), &hints, &ai_list)
      == 0);

  LOG4STRM_DEBUG 
    (Logging::Root (), 
    oss_ << "Create new RServerSocketAddress: "; outString (oss_)
    );
}

