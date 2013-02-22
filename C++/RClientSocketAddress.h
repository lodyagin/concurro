#pragma once
#include "RMultiprotoSocketAddress.h"

class RClientSocketAddress :
  public RMultiprotoSocketAddress
{
public:
  RClientSocketAddress 
    (const char* hostname,
     const char* port
    );

  // Overrides
  void outString (std::ostream& out) const;
};
