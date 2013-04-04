#include "StdAfx.h"
#include "RMultiprotoSocketAddress.h"

RMultiprotoSocketAddress::~RMultiprotoSocketAddress ()
{
#ifdef VIEW_AS_ADDRINFO
  delete aiw;
#endif
}

void RMultiprotoSocketAddress::init
  (const char *hostname, 
   const char *service, 
   const addrinfo& hints
   )
{
  addrinfo* res;

  LOG_DEBUG
    (Logger<LOG::Root>,
          "Call getaddrinfo (" 
          << hostname
          << ", [" << service << "], "
          << hints;
     );

  rSocketCheck
    (::getaddrinfo (hostname, service, &hints, &res)
     == 0);
#ifdef VIEW_AS_ADDRINFO
  aiw = new AddrinfoWrapper (res); // FIXME check alloc
#endif
}

void RMultiprotoSocketAddress::outString 
  (std::ostream& out) const
{
  std::ostream_iterator<RSingleprotoSocketAddress> 
	 os (out);

  std::copy (begin (), end (), os);
}

