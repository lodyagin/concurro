#include "StdAfx.h"
#include "ClientSocketConnector.h"

RConnectedSocket* ClientSocketConnector::connect_first 
  (const RClientSocketAddress& csa)
{
  RConnectedSocket* res = 0;
  SOCKET s = INVALID_SOCKET;

  for 
    (RClientSocketAddress::const_iterator cit =
      csa.begin ();
     cit != csa.end ();
     cit++)
  {
    sSocketCheck 
      ((s = ::socket 
        (cit->ai_family, 
         cit->ai_socktype,
         cit->ai_protocol)
        ) != INVALID_SOCKET
       );
    sSocketCheck
      (::connect (s, cit->ai_addr, cit->ai_addrlen)
       == 0
       );
    res = new RConnectedSocket (s, true);
    break; // FIXME
  }
  return res;
}
