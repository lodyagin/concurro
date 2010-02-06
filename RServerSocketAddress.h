#pragma once
#include "rsocketaddress.h"

  //FIXME RServerSocketAddress type
  // is incorrect. Should be replaced
  // with a set of RSocketAddress

// It is a "passive" socket address used by server
// to listen for connection.
class RServerSocketAddress : public RSocketAddress
{
public:
  RServerSocketAddress (unsigned int port);
  ~RServerSocketAddress ();
  void outString (std::ostream& out) const;

  // Overrides
  int get_port () const;

  // Overrides
  const std::string& get_ip () const;

  // Overrides
  /*void get_IPv4_sockaddr 
    (struct sockaddr* out, 
     int sockaddr_len_in,
     int* sockaddr_len_out
     ) const;*/

  // Overrides
  /*void get_sockaddr 
    (struct sockaddr* out, 
     int sockaddr_len_in,
     int* sockaddr_len_out
     ) const;*/

  // Overrides
  SockAddrList get_all_addresses () const;

protected:
  struct addrinfo *ai_list;
};
