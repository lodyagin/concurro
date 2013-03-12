#include "StdAfx.h"
#include "IPv4SocketAddress.h"
#ifdef _WIN32
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#else
#  include <arpa/inet.h>
#endif

IPv4SocketAddress::IPv4SocketAddress
  (const struct sockaddr* sa,
   int sa_len
   )
   : port (-1)
{
  assert (sa);

  if (sa->sa_family != AF_INET)
    THROW_EXCEPTION(SException, "Not IPv4 address");

  if (sa_len != sizeof (sa_in))
    THROW_EXCEPTION(SException, "Bad value of sa_len parameter");

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
  if (ip.length () == 0) {
#ifdef _WIN32
    ip = ::inet_ntoa (sa_in.sin_addr);
#else
	 char buf[(3+1)*4];
	 ip = ::inet_ntop(AF_INET, &sa_in, buf, sizeof(buf));
#endif
  }

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
