#pragma once
#include "RSingleprotoSocketAddress.h"
#ifdef _WIN32
#  include <Ws2tcpip.h>
#else
#  include <netinet/in.h>
#endif

class IPv6SocketAddress :
  public RSingleprotoSocketAddress
{
public:
  IPv6SocketAddress 
    (const struct sockaddr* sa,
     int sa_len
     );
  void outString (std::ostream& out) const;

  // Override
  int get_port () const;
  
  // Override
  const std::string& get_ip () const;

  // Override
  void get_sockaddr 
    (struct sockaddr* out, 
     int out_max_size,
     int* copied_size
     ) const;

protected:
  struct sockaddr_in6 sa_in;

  mutable int port;
  mutable std::string ip;
};
