#include "StdAfx.h"
#include "ConnectionPars.h"

RConnection* ConnectionPars::create_derivation
    (const ConnectionRepository::ObjectCreationInfo& info) const
{
  assert (socket);
  return new RConnection 
    (info.repository, 
     socket,
     info.objectId);
}
