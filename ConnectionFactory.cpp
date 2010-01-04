#include "StdAfx.h"
#include "ConnectionFactory.h"

ConnectionFactory::ConnectionFactory ()
:  Repository (2)
{
}


RConnection* ConnectionFactory::create_new_connection
  (RConnectedSocket* cs)
{
  assert (cs);
  const RSocketAddress& rsa = cs->get_peer_address ();
  LOG4STRM_DEBUG
    (Logging::Root (),
    oss_ << "Connection from: "
         << rsa.get_ip () << ':'
         << rsa.get_port ()
     );

  RConnection* rc = NULL;
  rc = create_object (cs);
  rc->start ();

  LOG4CXX_DEBUG 
    (Logging::Root (), 
    "New connection is started");
  return rc;
}
