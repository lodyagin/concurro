#include "StdAfx.h"
#include "IPv4SocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

IPv4SocketAddress::IPv4SocketAddress
  (const struct sockaddr* sa,
   int sa_len
   )
   : port (-1)
{
  assert (sa);

  if (sa->sa_family != AF_INET)
    THROW_EXCEPTION
      (SException, 
       oss_ << "Not IPv4 address"
       );

  if (sa_len != sizeof (sa_in))
    THROW_EXCEPTION
      (SException,
       oss_ << "Bad value of sa_len parameter");

  copy_sockaddr 
    ((struct sockaddr*) &sa_in,
     sizeof (sa_in),
     sa
     );
}

int IPv4SocketAddress::get_port () const
{
  if (port == -1)
    port = ::htons (sa_in.sin_port);

  return port;
}

const std::string& IPv4SocketAddress::get_ip 
() const
{
  if (ip.length () == 0)
    ip = ::inet_ntoa (sa_in.sin_addr);

  return ip;
}

void IPv4SocketAddress::get_sockaddr 
  (struct sockaddr* out, 
   int out_max_size,
   int* copied_size
   ) const
{
  copy_sockaddr 
   (out, 
    out_max_size, 
    (const sockaddr*) &sa_in,
    copied_size
    );
}

void IPv4SocketAddress::outString 
  (std::ostream& out) const
{
  RSocketAddress::outString (out, (const sockaddr*) &sa_in);
}
