#include "StdAfx.h"
#include "RInOutSocket.h"

void RInOutSocket::send (const std::string& str)
{
  const std::string::size_type strLen = str.length ();
  const int nBytesSent = ::send 
    (socket, str.c_str (), strLen, 0);
  sSocketCheck (nBytesSent != SOCKET_ERROR);
  if (nBytesSent < strLen)
    throw SException ("send sent less bytes than requested");
}

void RInOutSocket::receive 
  (std::string& out)
{
  const int msgLen = ::recv 
    (socket, buf, sizeof (buf), 0);
  sSocketCheck (msgLen != SOCKET_ERROR);
  out.assign (buf, msgLen);
}

