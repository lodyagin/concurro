#pragma once
#include "rsingleprotosocketaddress.h"

class UnknownprotoSocketAddress :
  public RSingleprotoSocketAddress
{
public:
  UnknownprotoSocketAddress
    (const struct sockaddr* _sa,
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
  SOCKADDR_STORAGE sa;

  const static std::string unknown_ip;
};
