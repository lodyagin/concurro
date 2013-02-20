#pragma once
#include "rSingleprotoSocketaddress.h"

class IPv4SocketAddress 
  : public RSingleprotoSocketAddress
{
public:
  IPv4SocketAddress 
    (const struct sockaddr* sa,
     int sa_len
     );
  void outString (std::ostream& out) const;

  // Overrides
  int get_port () const;
  
  // Overrides
  const std::string& get_ip () const;

  // Overrides
  //SockAddrList get_all_addresses () const;

  // Overrides
  void get_sockaddr 
    (struct sockaddr* out, 
     int out_max_size,
     int* copied_size
     ) const;

protected:
  struct sockaddr_in sa_in;

  mutable int port;
  mutable std::string ip;
};
