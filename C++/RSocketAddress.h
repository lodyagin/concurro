#pragma once
#include "HasStringView.h"
#include <string>

class RSocketAddress : public HasStringView
{
public:
  virtual ~RSocketAddress (void);

  // sockaddr pretty print
  static void outString 
    (std::ostream& out, 
     const struct sockaddr* sa
     );

  // in_addr pretty print
  static void outString 
    (std::ostream& out, 
     const struct in_addr* ia
     );

  // in6_addr pretty print
  static void outString 
    (std::ostream& out, 
     const struct in6_addr* ia
     );

  // Copy socket address
  // The size of information copied is defined by 
  // the 'in' structure type.
  // It throws an exception if sockaddr_out_size is
  // less than copied size (and do not copy in this case).
  // If sockaddr_in_size != NULL set it the the
  // size of the structure copied.
  static void copy_sockaddr 
    (struct sockaddr* out,
     int sockaddr_out_size,
     const struct sockaddr* in,
     int* sockaddr_in_size = 0
     );

  // Get the sockaddr length by its type.
  // Return 0 if the address family is unsupported
  static int get_sockaddr_len 
    (const struct sockaddr* sa);
};


