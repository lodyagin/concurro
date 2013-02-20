#pragma once

#include "RConnectedSocket.h"
#include "RConnection.h"
#include "Repository.h"

struct ConnectionPars;

typedef Repository<RConnection, ConnectionPars> 
  ConnectionRepository;

struct ConnectionPars
{
  RConnectedSocket* socket;

  virtual RConnection* create_derivation
    (const ConnectionRepository::ObjectCreationInfo&) const;
};
