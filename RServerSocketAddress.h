#pragma once
#include "rsocketaddress.h"

// It is a "passive" socket address used by server
// to listen for connection.
class RServerSocketAddress : public RSocketAddress
{
public:
  RServerSocketAddress (unsigned int port);
  ~RServerSocketAddress ();
  void outString (std::ostream& out) const;

  // Overrides
  void get_IPv4_sockaddr 
    (struct sockaddr* out, 
     int sockaddr_len_in,
     int* sockaddr_len_out
     ) const;

  // Overrides
  void get_sockaddr 
    (struct sockaddr* out, 
     int sockaddr_len_in,
     int* sockaddr_len_out
     ) const;

protected:
  struct addrinfo *ai_list;
};
