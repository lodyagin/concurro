#include "StdAfx.h"
#include "RInOutSocket.h"

void RInOutSocket::send (const std::string& str)
{
  const int strLen = str.length ();
  const int nBytesSent = ::send 
    (socket, str.c_str (), strLen, 0);
  sSocketCheck (nBytesSent != SOCKET_ERROR);
  if (nBytesSent < strLen)
    throw SException ("send sent less bytes than requested");
}
