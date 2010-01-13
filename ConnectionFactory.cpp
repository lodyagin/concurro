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
  const RSocketAddress& rsa = cs
    -> get_peer_address ();

  LOG4STRM_INFO
    (Logging::Root (),
    oss_ << "New connection from: "
         << rsa.get_ip () << ':'
         << rsa.get_port ()
     );

  RConnection* rc = NULL;
  ConnectionPars* cp = create_connection_pars(cs);
  rc = create_object (*cp);
  delete cp; //!
  rc->start ();

  LOG4CXX_DEBUG 
    (Logging::Root (), 
    "New connection is started");
  return rc;
}

ConnectionPars* ConnectionFactory::create_connection_pars 
  (RConnectedSocket* cs) const
{
  assert (cs);
  ConnectionPars* cp = new ConnectionPars;
  //FIXME check object creation
  cp->socket = cs;
  return cp;
}
