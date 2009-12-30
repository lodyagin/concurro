#pragma once
#include "rsocketaddress.h"

// It is a "passive" socket address used by server
// to listen for connection.
class RServerSocketAddress :
  public RSocketAddress
{
public:
  RServerSocketAddress (unsigned int port);
};
