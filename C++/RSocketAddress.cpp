#include "stdafx.h"
#include "RSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

RSocketAddress::~RSocketAddress (void)
{
}

void RSocketAddress::outString 
  (std::ostream& out, const struct sockaddr* sa)
{
  if (sa == NULL)
  {
    out << "<null>";
    return;
  }

  switch (sa->sa_family)
  {
    case AF_INET:
      {
        const struct sockaddr_in* sain = 
          (const struct sockaddr_in*) sa;

        out << "sockaddr_in ("
          << "sin_port = " << ::htons (sain->sin_port)
            << ", sin_addr = ";
        outString (out, &sain->sin_addr);
        out << ')';
      }
      break;
    case AF_INET6:
      {
        const struct sockaddr_in6* sain = 
          (const struct sockaddr_in6*) sa;

        out << "sockaddr_in6 ("
          << "sin6_port = " << ::htons (sain->sin6_port)
            << ", sin_addr = ";
        outString (out, &sain->sin6_addr);
        out << ')';
      }
      break;
    default:
      out << "sockaddr_xxx, sa_family = "
          << sa->sa_family;
  }
}

void RSocketAddress::outString 
  (std::ostream& out, const struct in_addr* ia)
{
  assert (ia);
  out << (int) ia->s_net << '.'
      << (int) ia->s_host << '.'
      << (int) ia->s_lh << '.'
      << (int) ia->s_impno;
}

void RSocketAddress::outString 
  (std::ostream& out, const struct in6_addr* ia)
{
  assert (ia);
  bool first = true;
  std::ios_base::fmtflags oldOpts = out.flags
    (std::ios_base::hex /*| std::ios_base::showpoint*/);

  out << '[';
  for (int i = 0; i < 8; i++)
  {
    if (!first)
      out << ':';
    else 
      first = false;
    out << std::setw (4) << std::setfill ('0') << ia->u.Word [i];
  }
  out << ']';
  out.flags (oldOpts);
}

void RSocketAddress::copy_sockaddr 
  (struct sockaddr* out,
   int sockaddr_out_size,
   const struct sockaddr* in,
   int* sockaddr_in_size)
{
  assert (out);
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

  /*case AF_BTH:
    return sizeof (SOCKADDR_BTH);*/

  default:
    return 0;
  }
}
