#ifndef CONCURRO_SOCKETADDRESSFACTORY_H_
#define CONCURRO_SOCKETADDRESSFACTORY_H_

#include "RSingleprotoSocketAddress.h"
#ifdef _WIN32
#  include <winsock2.h>
#  include <Ws2tcpip.h>
typedef socklen_t int;
#else
#  include <sys/socket.h>
#  define SOCKADDR_STORAGE struct sockaddr_storage
#endif

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

  socklen_t* buffer_len_ptr ()
  {
    return &len;
  }

  // Create appropriate type of socket address
  RSingleprotoSocketAddress* create_socket_address ();

protected:
  SOCKADDR_STORAGE buf;
  socklen_t len;
};

#endif
