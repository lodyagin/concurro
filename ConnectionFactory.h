#pragma once
#include "SSingleton.h"
#include "SEvent.h"
#include "Repository.h"
#include "ThreadRepository.h"
#include "RConnectedSocket.h"

/*
All decisions about connection creation
are made by this class.
*/

// ConnectionFactory delegates all calls to
// ThreadRepository (threads)

template<class Connection, class ConnectionPars>
class ConnectionFactory :
  public SSingleton
    <ConnectionFactory<Connection, ConnectionPars>>
{
public:
  ConnectionFactory(ThreadRepository<Connection, ConnectionPars>* _threads)
    : threads (_threads),
      connectionTerminated (false) // automatic reset
  {
    assert (threads);
  }

  Connection* 
    create_new_connection (RConnectedSocket* cs);

  void destroy_terminated_connections ();

  SEvent connectionTerminated;

  typedef ThreadRepository<Connection, ConnectionPars> 
    ConnectionRepository;

protected:
  virtual ConnectionPars*
    create_connection_pars 
      (RConnectedSocket* cs);

  ConnectionRepository* threads;
};

template<class Connection, class ConnectionPars>
Connection* 
ConnectionFactory<Connection, ConnectionPars>::create_new_connection
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

  Connection* rc = NULL;
  ConnectionPars* cp = create_connection_pars(cs);
  rc = threads->create_object (*cp);
  delete cp; //!
  rc->start ();

  LOG4CXX_DEBUG 
    (Logging::Root (), 
    "New connection is started");
  return rc;
}

template<class Connection, class ConnectionPars>
ConnectionPars* 
ConnectionFactory<Connection, ConnectionPars>::create_connection_pars 
  (RConnectedSocket* cs)
{
  assert (cs);
  ConnectionPars* cp = 
    new ConnectionPars;
  //FIXME check object creation
  cp->socket = cs;
  cp->connectionTerminated = &connectionTerminated;
  return cp;
}

template<class Connection, class ConnectionPars>
void ConnectionFactory<Connection, ConnectionPars>::
  destroy_terminated_connections ()
{
    // Found and destroy the terminated thread
    std::list<int> terminated;
    threads->get_object_ids_by_state
      (std::back_inserter (terminated),
       SThread::terminatedState
       );
    std::for_each 
      (terminated.begin (), terminated.end (),
      ConnectionRepository::Destroy (*threads));
}
