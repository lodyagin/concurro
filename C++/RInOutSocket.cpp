#include "StdAfx.h"
#include "RInOutSocket.h"

int RInOutSocket::send (void* data, int len, int* error)
{
    int lenSent = ::send(socket, (const char*) data, len, 0);
		if (lenSent == -1) {
      const int err = ::WSAGetLastError ();
			if (err == WSAEINTR || 
			    err == WSAEWOULDBLOCK)
      {
        if (err == WSAEWOULDBLOCK) waitFdWrite = true;
        *error = err;
				return -1;
      }
      THROW_EXCEPTION
        (SException,
         oss_ << L"Write failed: " << sWinErrMsg (err));
		}
    *error = 0;
    return lenSent;
}

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

