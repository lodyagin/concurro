#include "StdAfx.h"
#include "RConnectedSocket.h"
#include "SocketAddressFactory.h"
#include "RConnection.h"
#include "RClientSocketAddress.h"

RConnectedSocket::RConnectedSocket 
  (SOCKET con_socket, bool withEvent)
   : RInOutSocket (con_socket, withEvent), peer (0)
{
  waitFdWrite = true;
}

/*RConnectedSocket::RConnectedSocket 
  (const RClientSocketAddress& csa)
{
  for 
    (RClientSocketAddress::const_iterator it = 
         csa.begin ();
     it != csa.end ();
     it++)
   {
     ::connect (
   }
}*/

RConnectedSocket::~RConnectedSocket ()
{
  delete peer;
}

const RSingleprotoSocketAddress& RConnectedSocket::
  get_peer_address ()
{
  if (!peer)
  {
    SocketAddressFactory saf;
    rSocketCheck (::getpeername 
      (socket, 
       saf.buffer (),
       saf.buffer_len_ptr ()
       ) == 0);
    peer = saf.create_socket_address ();
  }
  return *peer;
}