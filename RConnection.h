#pragma once
#include "sthread.h"
#include "RConnectedSocket.h"
#include "Repository.h"

class RConnection : public SThread
{ //TODO add states
public:
  // Usually it is called by ConnectionFactory
  RConnection 
    (void* repo, RConnectedSocket* cs);
  ~RConnection ();

  void identify_peer ();
protected:
  void run ();
  RConnectedSocket* socket;

  void* repository;
};
