#include "StdAfx.h"
#include "RSocketGroup.h"
#include <algorithm>

using namespace std;

struct ShutdownSocket : std::unary_function<SOCKET, void>
{
  void operator() (SOCKET s) 
  { 
    ::shutdown (s, SD_BOTH); 
    ::closesocket (s);
  }
};

class SetBlockingSocket : std::unary_function<SOCKET, void>
{
public:
  SetBlockingSocket (bool blocking) 
  {
    mode = (blocking) ? 0 : 1; 
  }
  void operator() (SOCKET s) 
  {  
    ioctlsocket(s, FIONBIO, &mode);
  }
private:
  u_long mode;
};

RSocketGroup::RSocketGroup ()
{
}

RSocketGroup::RSocketGroup (const Group& group)
: sockets (group)
{
}

RSocketGroup::~RSocketGroup ()
{
  for_each 
   (sockets.begin (),
    sockets.end (),
    ShutdownSocket ()
    );
}

void RSocketGroup::set_blocking (bool blocking)
{
  std::for_each
    (sockets.begin (), sockets.end (),
     SetBlockingSocket (blocking)
     );
}

