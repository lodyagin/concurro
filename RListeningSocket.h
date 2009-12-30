#pragma once
#include "RSocket.h"
#include "RServerSocketAddress.h"
#include "ConnectionFactory.h"
#include "SThread.h"
#include <winsock2.h>

class RListeningSocket : public RSocket
{
public:
  // Create and bind the server socket
  RListeningSocket 
    (const RServerSocketAddress& addr);

  // Listen and generate new connections
  void listen 
    (unsigned int backlog,
     ConnectionFactory& cf);

protected:
  SOCKET socket;

  // Called by constructor
  void bind (const RServerSocketAddress& addr);
};
