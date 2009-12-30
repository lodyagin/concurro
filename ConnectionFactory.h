#pragma once
#include "RConnection.h"
#include "SSingleton.h"
#include "Repository.h"
#include "RConnectedSocket.h"

typedef Repository<RConnection, RConnectedSocket*> 
  ConnectionRepository;

class ConnectionFactory :
  public SSingleton<ConnectionFactory>,
  protected ConnectionRepository
{
public:
  ConnectionFactory ();
  //~ConnectionFactory ();
  RConnection* create_new_connection
    (RConnectedSocket* cs);
};
