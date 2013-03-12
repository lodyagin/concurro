#include "StdAfx.h"
#include "SocketAddressFactory.h"
#include "IPv4SocketAddress.h"
#include "IPv6SocketAddress.h"

RSingleprotoSocketAddress* 
SocketAddressFactory::create_socket_address ()
{
  switch (((sockaddr*) &buf)->sa_family)
  {
  case AF_INET:
    if (len != RSocketAddress::get_sockaddr_len 
      ((sockaddr*) &buf)
      )
      THROW_EXCEPTION(SException, "Invalid sockaddr length");
    return new IPv4SocketAddress 
      ((const sockaddr*) &buf, len);

  case AF_INET6:
    if (len != RSocketAddress::get_sockaddr_len 
      ((sockaddr*) &buf)
      )
      THROW_EXCEPTION(SException, "Invalid sockaddr length");
    return new IPv6SocketAddress 
      ((const sockaddr*) &buf, len);

  default:
    THROW_EXCEPTION
      (SException,
		 SFORMAT("Unsupported socket family: "
					<< ((sockaddr*) &buf)->sa_family)
      );
  }
}

