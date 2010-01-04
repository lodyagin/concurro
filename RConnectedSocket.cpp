#include "StdAfx.h"
#include "RConnectedSocket.h"
#include "SocketAddressFactory.h"

RConnectedSocket::RConnectedSocket (SOCKET con_socket)
   : RInOutSocket (con_socket), peer (0)
{
}

RConnectedSocket::~RConnectedSocket ()
{
  delete peer;
}


const RSocketAddress& RConnectedSocket::get_peer_address ()
{
  if (!peer)
  {
    SocketAddressFactory saf;
    sSocketCheck (::getpeername 
      (socket, 
       saf.buffer (),
       saf.buffer_len_ptr ()
       ) == 0);
    peer = saf.create_socket_address ();
  }
  return *peer;
}
