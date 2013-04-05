// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#include "RSocketAddress.h"
#ifdef _WIN32
#  include <winsock2.h>
#  include <Ws2tcpip.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#endif

std::ostream& operator << 
  (std::ostream& out, const addrinfo& ai)
{
  out << "addrinfo ("
      << "ai_flags: "  << std::hex << "0x" << ai.ai_flags
      << std::dec 
      << " ai_family: " << ai.ai_family
      << " ai_socktype: " << ai.ai_socktype
      << " ai_protocol: " << ai.ai_protocol
      << " ai_canonname: [" 
      << ((ai.ai_canonname) ? ai.ai_canonname : "<null>")
      << "] ai_addr: ";
  RSocketAddress::outString (out, ai.ai_addr);
  out << ")\n";
  return out;
}

size_t AddressRequestBase
//
::n_objects(const ObjectCreationInfo& oi)
{
  addrinfo* ai_;

  rSocketCheck(
    getaddrinfo(host.c_str(),
                SFORMAT(port).c_str(), 
                &hints, &ai_) 
    == 0);
  aw_ptr = std::make_shared<AddrinfoWrapper>(ai_);
  next_obj = aw_ptr->begin();
  return aw_ptr->size();
}

RSocketAddress* AddressRequestBase
//
::create_next_derivation(const ObjectCreationInfo& oi)
{
  return new RSocketAddress(oi, aw_ptr, &*next_obj++);
}

AddrinfoWrapper::AddrinfoWrapper (addrinfo* _ai)
  : ai (_ai), theSize (0)
{
  // Count the size
  for (addrinfo* ail = ai; ail != 0; ail = ail->ai_next)
    theSize++;
}

AddrinfoWrapper::~AddrinfoWrapper ()
{
  if (ai)
    ::freeaddrinfo (ai);
}

RSocketAddress::RSocketAddress
  (const ObjectCreationInfo& oi,
   const std::shared_ptr<AddrinfoWrapper>& ptr,
	const addrinfo* ai_)
:   StdIdMember(oi.objectId),
    ai(ai_),
	 aw_ptr(ptr)
{
  assert(ai);
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
#ifdef _WIN32
  out << (int) ia->s_net << '.'
      << (int) ia->s_host << '.'
      << (int) ia->s_lh << '.'
      << (int) ia->s_impno;
#else
  char buf[(3+1)*4];
  out << inet_ntop(AF_INET, ia, buf, sizeof(buf));
#endif
}

void RSocketAddress::outString 
  (std::ostream& out, const struct in6_addr* ia)
{
  assert (ia);
#ifdef _WIN32
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
#else
  char buf[(4+1)*8];
  out << inet_ntop(AF_INET6, ia, buf, sizeof(buf));
#endif
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
      (SException, SFORMAT(
              "Too little space for copy sockaddr: "
           << "there is " << sockaddr_out_size
           << " bytes but " << in_size
           << " bytes required."));

  //FIXME allow invalid parameter handle
#ifdef _WIN32
  ::memcpy_s (out, sockaddr_out_size, in, in_size);
#else
  ::memcpy(out, in, sockaddr_out_size);
#endif

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

std::ostream&
operator<< (std::ostream& out, const RSocketAddress& sa)
{
  RSocketAddress::outString(out, sa.ai->ai_addr);
  return out;
}
