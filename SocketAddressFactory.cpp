#include "StdAfx.h"
#include "SocketAddressFactory.h"
#include "IPv4SocketAddress.h"

RSocketAddress* SocketAddressFactory::create_socket_address
  ()
{
  switch (((sockaddr*) &buf)->sa_family)
  {
  case AF_INET:
    if (len != RSocketAddress::get_sockaddr_len 
      ((sockaddr*) &buf)
      )
      THROW_EXCEPTION
        (SException,
         oss_ << "Invalid sockaddr length");
    return new IPv4SocketAddress 
      ((const sockaddr*) &buf, len);

  default:
    THROW_EXCEPTION
      (SException,
      oss_ << "Unsupported socket family: "
           << ((sockaddr*) &buf)->sa_family
      );
  }
}

