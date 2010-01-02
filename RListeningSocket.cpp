#include "stdafx.h"
#include "RListeningSocket.h"
#include "RConnectedSocket.h"

//FIXME! close sockets on errors
RListeningSocket::RListeningSocket
  (const RServerSocketAddress& addr)
{
  socket = ::socket 
    (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  sSocketCheck (socket != INVALID_SOCKET);

  bind (addr);
}

void RListeningSocket::bind 
  (const RServerSocketAddress& addr)
{
  const sockaddr* saddr = NULL;
  int saddr_len = 0;

  // Set reuse address
  const int on = 1;
  sSocketCheck 
    (::setsockopt
      (socket,
       SOL_SOCKET,
       SO_REUSEADDR,
       (const char*) &on,
       sizeof (on)
       ) == 0
     );
  addr.get_IPv4_sockaddr (&saddr, &saddr_len);
  ::bind (socket, saddr, saddr_len);
}

void RListeningSocket::listen
    (unsigned int backlog,
     ConnectionFactory& cf)
{
  //TODO keepalive option

  sSocketCheck (::listen (socket, (int) backlog) == 0);

  struct sockaddr sa;
  int sa_len = sizeof (sa);
  set_blocking (false);
  while (1) // loop for producing new connections
  {
    SOCKET s = 0;
    while (1) // loop for waiting connection
    {
      if (SThread::current ().is_stop_requested ())
        ::xShuttingDown 
          ("Stop request from the owner thread.");

      s = ::accept (socket, &sa, &sa_len); 
      //immediate returns

      if (SThread::current ().is_stop_requested ()) 
        return; // thread stop is requested

      if (s == WSAEWOULDBLOCK)
        ::Sleep (1000); // TODO
      else {
        if (s == INVALID_SOCKET) {
          if (int err = WSAGetLastError())
            throw SException
              ("Error : " 
               + sWinErrMsg(err)
               );
          else
            ::Sleep (1000); // TODO
        }
        else break;
      }
    }

    LOG4STRM_DEBUG 
      (Logging::Root (), 
       oss_ << "accept returns the new connection: ";
       RSocketAddress::outString (oss_, &sa)
       );
    cf.create_new_connection 
      (new RConnectedSocket (s));
  }
}
