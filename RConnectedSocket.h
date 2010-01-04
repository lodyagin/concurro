#pragma once
#include "rinoutsocket.h"

class RConnectedSocket : public RInOutSocket
{
public:
  // It is usually called only from RListeningSocket
  RConnectedSocket (SOCKET con_socket);

  // Return the address of the peer's socket.
  //RSocketAddress get_peer_address ();
};
