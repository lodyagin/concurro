#include "StdAfx.h"
#include "RSocket.h"

RSocket::~RSocket ()
{
  if (socket)
    ::shutdown (socket, SD_BOTH);
}

void RSocket::set_blocking (bool blocking)
{
  u_long mode = (blocking) ? 0 : 1;
  ioctlsocket(socket, FIONBIO, &mode);
}
