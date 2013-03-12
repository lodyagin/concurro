#include "StdAfx.h"
#include "RInOutSocket.h"
#ifndef _WIN32
#  include <sys/socket.h>
#  define WSAEINTR EINTR
#  define WSAEWOULDBLOCK EWOULDBLOCK
#endif

int RInOutSocket::send (void* data, int len, int* error)
{
    int lenSent = ::send(socket, (const char*) data, len, 0);
		if (lenSent == -1) {
#ifdef _WIN32
      const int err = ::WSAGetLastError ();
#else
      const int err = errno;
#endif
			if (err == WSAEINTR || 
			    err == WSAEWOULDBLOCK)
      {
        if (err == WSAEWOULDBLOCK) waitFdWrite = true;
        *error = err;
				return -1;
      }
      THROW_EXCEPTION(SException,
							 SFORMAT("Write failed: " << strerror(err)));
		}
    *error = 0;
    return lenSent;
}

#if 0 // FIX sleep
size_t RInOutSocket::atomicio_send (void* data, size_t n)
{
	char *s = reinterpret_cast<char*> (data);
	size_t pos = 0;
	int res, error;

	while (n > pos) {
		res = this->send (s + pos, n - pos, &error);
    if (error)
    {
	    if (error == WSAEINTR)
		    continue;

      if (error == WSAEWOULDBLOCK) {
          ::Sleep(1000); // FIXME
				  continue;
			}

      return 0;
    }

    if (res == 0)
      THROW_EXCEPTION
        (SException,
         oss_ << L"The connection is closed by peer");

  	pos += (size_t)res;
	}
	return (pos);
}
#endif
