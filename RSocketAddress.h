#pragma once
#include "HasStringView.h"
#include <string>

class RSocketAddress : public HasStringView
{
  friend class RListeningSocket;  
  // for access to sockaddr components
  // of an internal ai_list

public:
  RSocketAddress () : ai_list (NULL) {}
  virtual ~RSocketAddress (void);
  void outString (std::ostream& out) const;

  // addrinfo pretty print
  static void outString (std::ostream& out, const struct addrinfo* ai);

  // sockaddr pretty print
  static void outString (std::ostream& out, const struct sockaddr* sa);

  // in_addr pretty print
  static void outString (std::ostream& out, const struct in_addr* ia);

protected:
  // return the first IPv4 socket address
  // It is protected because live of sockaddr is 
  // bound to this object live, only friends can access
  // directly.
  void get_IPv4_sockaddr 
    (const struct sockaddr** out, 
     int* sockaddr_len) const;

  struct addrinfo *ai_list;
};


