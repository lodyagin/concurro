#pragma once
#include "rinoutsocket.h"
#include "RSocketAddress.h"

class RConnectedSocket : public RInOutSocket
{
public:
  // It is usually called only from RListeningSocket
  RConnectedSocket (SOCKET con_socket);
  ~RConnectedSocket ();

  // Return the address of the peer's socket.
  const RSocketAddress& get_peer_address ();
protected:
  RSocketAddress* peer;
};
