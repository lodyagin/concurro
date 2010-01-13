#include "StdAfx.h"
#include "ConnectionPars.h"

ConnectionPars::~ConnectionPars(void)
{
}

RConnection* ConnectionPars::create_derivation
  (void* repo) const
{
  assert (socket);
  return new RConnection (repo, socket);
}
