#pragma once
#include "rsocketaddress.h"

class IPv4SocketAddress : public RSocketAddress
{
public:
  IPv4SocketAddress 
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
  struct sockaddr_in sa_in;

  mutable int port;
  mutable std::string ip;
};
