#pragma once
#include "rinoutsocket.h"

class RConnectedSocket : public RInOutSocket
{
public:
  // It is usually called only from RListeningSocket
  RConnectedSocket (SOCKET con_socket);
};
