#include "stdafx.h"
#include "RServerSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <Ws2bth.h>

RServerSocketAddress::RServerSocketAddress 
  (unsigned int port)
  : ai_list (NULL)
{
  struct addrinfo hints = {0};

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  //hints.ai_protocol = IPPROTO_TCP;

  const char* hostname = NULL;

  // make a string representation of a port
  std::string portStr;
  ::toString (port, portStr);

  LOG4STRM_DEBUG
    (Logging::Root (),
    oss_ << "Call getaddrinfo (" 
         << ((hostname) ? hostname : NULL)
         << ", [" << portStr.c_str () << "], ";
    RSocketAddress::outString (oss_, &hints)
     );

  sSocketCheck //FIXME gai_strerror must be used for formatting an error
    (::getaddrinfo 
      (hostname, portStr.c_str (), &hints, &ai_list)
      == 0);

  LOG4STRM_DEBUG 
    (Logging::Root (), 
    oss_ << "Create new RServerSocketAddress: "; outString (oss_)
    );
}

RServerSocketAddress::~RServerSocketAddress ()
{
  if (ai_list != NULL)
    ::freeaddrinfo (ai_list);
}

int RServerSocketAddress::get_port () const 
{
  THROW_EXCEPTION(SException, oss_ << "Unimplemented");
}

const std::string& RServerSocketAddress::get_ip () const
{
  THROW_EXCEPTION(SException, oss_ << "Unimplemented");
}

#if 0
void RServerSocketAddress::get_IPv4_sockaddr 
  (struct sockaddr* out, 
   int out_max_size,
   int* copied_size
  ) const
{
  assert (copied_size); // must be provided

  const struct addrinfo *ai = ai_list;

  while (ai != NULL && ai->ai_family != AF_INET)
    ai = ai->ai_next;

  if (ai == NULL)
    throw SException ("No IPv4 socket available");

  copy_sockaddr 
    (out, out_max_size, ai->ai_addr, copied_size);
}

void RServerSocketAddress::get_sockaddr 
  (struct sockaddr* out, 
   int out_max_size,
   int* copied_size
   ) const
{
  get_IPv4_sockaddr 
    (out, out_max_size, copied_size);
}
#endif

RServerSocketAddress::SockAddrList 
RServerSocketAddress::get_all_addresses () const
{
  SockAddrList res;
  for (const addrinfo* ai = ai_list;
       ai != 0;
       ai = ai->ai_next)
  {
    size_t sa_len = get_sockaddr_len (ai->ai_addr);
    if (sa_len)
    {
#if 0
      void* sa = malloc (sa_len);
      // FIXME check alloc
      copy_sockaddr 
        ((sockaddr*) sa, sa_len, ai->ai_addr, 0);
      res.push_back ((sockaddr*) sa);
#else
      res.push_back (ai);
#endif
    }
  }
  return res;
}

void RServerSocketAddress::outString (std::ostream& out) const
{
  const struct addrinfo *ai = ai_list;

  out << "RSocketAddress:\n";
  while (ai != NULL)
  {
    RSocketAddress::outString (out, ai);
    out << '\n';

    ai = ai->ai_next;
  }
  out << '\n';
}

