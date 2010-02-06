#pragma once
#include "RSocketGroup.h"
#include "RServerSocketAddress.h"
#include "ConnectionFactory.h"
#include "SThread.h"
#include <winsock2.h>
#include "Logging.h"

class RListeningSocket : public RSocketGroup
{
public:
  // Create and bind the server socket
  RListeningSocket 
    (const RServerSocketAddress& addr,
     unsigned int backlog);

  ~RListeningSocket ();

  // Listen and generate new connections
  void listen (ConnectionFactory& cf);

protected:

  // Called by constructor
  void bind (const RServerSocketAddress& addr,
             unsigned int backlog);

  WSAEVENT *events;

private:
  static Logging log;
};
