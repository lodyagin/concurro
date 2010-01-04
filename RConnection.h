#pragma once
#include "sthread.h"
#include "RConnectedSocket.h"
#include "Repository.h"
#include "Logging.h"

class RConnection : public SThread
{ //TODO add states
public:
  
  // RConnection takes the socket ownership
  // and will destroy it.
  static RConnection* create
    (void* repo, RConnectedSocket* cs);

protected:
  // Usually it is called by ConnectionFactory
  RConnection 
    (void* repo, RConnectedSocket* cs);
  ~RConnection ();

  void run ();

  RConnectedSocket* socket;
  void* repository;
private:
  static Logging log;
};
