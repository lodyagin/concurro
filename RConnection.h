#pragma once

#include "sthread.h"
#include "RConnectedSocket.h"
#include "Repository.h"
#include "Logging.h"

struct ConnectionPars;

class RConnection : public SThread
{ //TODO add states
  friend ConnectionPars;

public:
  
  RConnectedSocket* get_socket ()
  {
    assert (socket);
    return socket;
  }

protected:
  // Usually it is called by ConnectionFactory
  // RConnection takes the socket ownership
  // and will destroy it.
  RConnection 
    (void* repo, RConnectedSocket* cs);
  ~RConnection ();

  //void run ();

  RConnectedSocket* socket;
  void* repository;
private:
  static Logging log;
};
