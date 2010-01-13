#pragma once
#include "RConnection.h"
#include "SSingleton.h"
#include "Repository.h"
#include "ConnectionPars.h"
#include "RConnectedSocket.h"

typedef Repository<RConnection, ConnectionPars> 
  ConnectionRepository;

/*
All decisions about connection creation
are made by this class.
*/

class ConnectionFactory :
  public SSingleton<ConnectionFactory>,
  protected ConnectionRepository
{
public:
  ConnectionFactory ();

  RConnection* create_new_connection
    (RConnectedSocket* cs);

protected:

  virtual ConnectionPars* create_connection_pars 
    (RConnectedSocket* cs) const;
};
