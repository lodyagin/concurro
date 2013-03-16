#pragma once
#include "RInOutSocket.h"
#include "RSingleprotoSocketAddress.h"

/// This object is always created in "connected" state.
class RConnectedSocket : public RInOutSocket
{
public:
  // It is usually called only from RListeningSocket
  RConnectedSocket (SOCKET con_socket, bool withEvent);
  
  ~RConnectedSocket ();

  // Return the address of the peer's socket.
  const RSingleprotoSocketAddress& get_peer_address ();
protected:
  RSingleprotoSocketAddress* peer;
};
