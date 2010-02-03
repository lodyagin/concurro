#include "StdAfx.h"
#include "RConnectedSocket.h"
#include "SocketAddressFactory.h"
#include "RConnection.h"

RConnectedSocket::RConnectedSocket (SOCKET con_socket, bool withEvent)
   : RInOutSocket (con_socket, withEvent), peer (0)
{
  waitFdWrite = true;
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
