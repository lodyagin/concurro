#pragma once
#include "RMultiprotoSocketAddress.h"

// It is a "passive" socket address used by server
// to listen for connection.
class RServerSocketAddress 
  : public RMultiprotoSocketAddress
{
public:
  RServerSocketAddress (unsigned int port);

  // Overrides
  void outString (std::ostream& out) const;
};
