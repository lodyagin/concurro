#pragma once
#include "RClientSocketAddress.h"
#include "RConnectedSocket.h"

class ClientSocketConnector
{
public:
  ClientSocketConnector () {}
  virtual ~ClientSocketConnector () {}

  // Return th connection to the first address
  // to which it is connected successfully
  RConnectedSocket* connect_first 
    (const RClientSocketAddress& csa);
};
