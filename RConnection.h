#pragma once
#include "sthread.h"
#include "RConnectedSocket.h"
#include "Repository.h"

class RConnection : public SThread
{
public:
  // Usually it is called by ConnectionFactory
  RConnection 
    (void* repo, RConnectedSocket* cs);

  ~RConnection ();
protected:
  void run ();
  RConnectedSocket* socket;

  void* repository;
};
