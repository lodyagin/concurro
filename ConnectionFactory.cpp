#include "StdAfx.h"
#include "ConnectionFactory.h"

ConnectionFactory::ConnectionFactory ()
:  Repository (2)
{
}


RConnection* ConnectionFactory::create_new_connection
  (RConnectedSocket* cs)
{
  RConnection* rc = NULL;
  rc = create_object (cs);
  rc->start ();
  LOG4CXX_DEBUG 
    (Logging::Root (), 
    "New connection is started");
  return rc;
  //catch (const NoMoreObjectsPossible&) {}
  /*catch (...) 
  {
    LOG4CXX_WARN 
      (Logging::Root (), "Unknown exception");
  }*/

  return rc;
}
