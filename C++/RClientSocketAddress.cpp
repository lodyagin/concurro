#include "StdAfx.h"
#include "RClientSocketAddress.h"

RClientSocketAddress::RClientSocketAddress 
  (const char* hostname,
   const char* port
   )
{
  assert (port);
  struct addrinfo hints = {0};

  hints.ai_family = AF_UNSPEC;
  //hints.ai_socktype = SOCK_STREAM;

  init (hostname, port, hints);

  LOG_DEBUG 
    (Logger<LOG::Root>, 
	  "New RClientSocketAddress is created: " << *this
    );
}

void RClientSocketAddress::outString 
  (std::ostream& out) const
{
  out << "RClientSocketAddress:\n";
  RMultiprotoSocketAddress::outString (out);
  out << '\n';
}

