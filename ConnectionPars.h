#pragma once

#include "RConnectedSocket.h"
#include "RConnection.h"

struct ConnectionPars
{
  RConnectedSocket* socket;
  virtual ~ConnectionPars(void);

  virtual RConnection* create_derivation
    (void* repo) const;
};
