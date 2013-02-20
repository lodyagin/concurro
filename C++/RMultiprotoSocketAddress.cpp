#include "StdAfx.h"
#include "RMultiprotoSocketAddress.h"

std::ostream& operator << 
  (std::ostream& out, const addrinfo& ai)
{
  out << "addrinfo ("
      << "ai_flags: "  << ai.ai_flags
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

RMultiprotoSocketAddress::~RMultiprotoSocketAddress ()
{
  delete aiw;
}

void RMultiprotoSocketAddress::init
  (const char *hostname, 
   const char *service, 
   const addrinfo& hints
   )
{
  addrinfo* res;

  LOG4STRM_DEBUG
    (Logging::Root (),
     oss_ << "Call getaddrinfo (" 
          << hostname
          << ", [" << service << "], "
          << hints;
     );

  sSocketCheck
    (::getaddrinfo (hostname, service, &hints, &res)
     == 0);
  aiw = new AddrinfoWrapper (res); // FIXME check alloc
}

void RMultiprotoSocketAddress::outString 
  (std::ostream& out) const
{
  std::ostream_iterator<addrinfo> os (out);

  std::copy (begin (), end (), os);
}

