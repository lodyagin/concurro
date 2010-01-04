#include "stdafx.h"
#include "RServerSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

RServerSocketAddress::RServerSocketAddress 
  (unsigned int port)
  : ai_list (NULL)
{
  struct addrinfo hints = {0};

  hints.ai_family = AF_INET; //TODO set AF_UNSPEC
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

void RSocketAddress::copy_sockaddr 
  (struct sockaddr* out,
   int sockaddr_out_size,
   const struct sockaddr* in,
   int* sockaddr_in_size)
{
  //assert (out);
  const int in_size = get_sockaddr_len (in);
  if (sockaddr_out_size < in_size)
    THROW_EXCEPTION
      (SException,
      oss_ << "Too little space for copy sockaddr: "
           << "there is " << sockaddr_out_size
           << " bytes but " << in_size
           << " bytes required.");

  //FIXME allow invalid parameter handle
  ::memcpy_s (out, sockaddr_out_size, in, in_size);

  if (sockaddr_in_size)
    *sockaddr_in_size = in_size;
}


int RSocketAddress::get_sockaddr_len (const struct sockaddr* sa)
{
  assert (sa);
  switch (sa->sa_family)
  {
  case AF_INET:
    return sizeof (/*struct */ sockaddr_in);

  case AF_INET6:
    return sizeof (struct sockaddr_in6);

  default:
    THROW_EXCEPTION
      (SException, 
       oss_ << "Unknown socket type: "
        << sa->sa_family
        );
  }
}
