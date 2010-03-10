#pragma once
#include "RSingleprotoSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

// 1. Get pointer to buffer by buffer () call
// 2. Fill buffer
// 3. Get the address by create_socket_address call

class SocketAddressFactory
{
public: // TODO add states
  SocketAddressFactory ()
    : len (sizeof (buf))
  {}

  // Return pointers to the buffer, which
  // can be used to socket function output
  sockaddr* buffer ()
  {
    return (sockaddr*) &buf;
  }

  int* buffer_len_ptr ()
  {
    return &len;
  }

  // Create appropriate type of socket address
  RSingleprotoSocketAddress* create_socket_address ();

protected:
  SOCKADDR_STORAGE buf; 
  int len;
};
