#include "StdAfx.h"
#include "RConnection.h"
#include "..\Server\Options.h" //FIXME
#include "ConnectionFactory.h"

RConnection::RConnection 
  (void* repo, 
   RConnectedSocket* cs
   ) : socket (cs), repository (repo)
{
  assert (repo);
  assert (socket);
}

RConnection::~RConnection ()
{
}

void RConnection::run ()
{
  socket->send 
    (Options::instance ()
      .get_protocol_version_exchange_string ()
      );
  for (int i = 0; i < 25; i++) //FIXME to options
  {
    if (SThread::current ().is_stop_requested ()) 
      break; // thread stop is requested

    ::Sleep (1000); 
  }

  delete socket;
  socket = NULL; //TODO add UT checking working
  // with socket-null objects

  ((ConnectionRepository*)repository)->delete_object 
    (this, false); // false means not to delete this
}