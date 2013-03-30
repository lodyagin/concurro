// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef CONCURRO_RSOCKETGROUP_H_
#define CONCURRO_RSOCKETGROUP_H_

#include "RSocket.h"
#include <vector>

class RSocketGroup :
  virtual public RSocket
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

#endif
