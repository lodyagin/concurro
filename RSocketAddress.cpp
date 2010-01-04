#include "stdafx.h"
#include "RSocketAddress.h"
#include <winsock2.h>
#include <Ws2tcpip.h>

RSocketAddress::~RSocketAddress (void)
{
}

void RSocketAddress::outString 
  (std::ostream& out, const struct addrinfo* ai)
{
  if (ai == NULL)
  {
    out << "<null>";
    return;
  }

  out << "addrinfo ("
      << "ai_flags: "  << ai->ai_flags
      << " ai_family: " << ai->ai_family
      << " ai_socktype: " << ai->ai_socktype
      << " ai_protocol: " << ai->ai_protocol
      << " ai_canonname: [" 
      << ((ai->ai_canonname) ? ai->ai_canonname : "<null>")
      << "] ai_addr: ";
  outString (out, ai->ai_addr);
  out << ')';
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
      }
      break;
    default:
      out << "sockaddr_xxx, sa_family = "
          << sa->sa_family << ')';
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
