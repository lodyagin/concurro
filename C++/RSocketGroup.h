#pragma once
#include "rsocket.h"
#include <vector>

class RSocketGroup :
  public RSocket
{
public:
  typedef std::vector<SOCKET> Group;

  RSocketGroup (const Group& group);
  ~RSocketGroup ();

protected:
  RSocketGroup ();

  // Overrides
  void set_blocking (bool blocking);


  Group sockets;
};
