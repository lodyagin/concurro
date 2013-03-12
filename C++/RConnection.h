/*
It is a connection created by the socket.
*/

#ifndef CONCURRO_RCONNECTION_H_
#define CONCURRO_RCONNECTION_H_

#include "RThread.h"
#include "RConnectedSocket.h"
#include "Logging.h"

template<class Thread>
class RConnection : public Thread
{ //TODO add states
public:

  const std::string universal_object_id;
  
  RConnectedSocket* get_socket ()
  {
    assert (socket);
    return socket;
  }

  ~RConnection ()
  {
    delete socket;
  }

protected:
  // Usually it is called by ConnectionFactory
  // RConnection takes the socket ownership
  // and will destroy it.
  RConnection 
    (void* repo, 
     RConnectedSocket* cs,
     const std::string& objId,
     const typename Thread::ConstrPar& par,
     REvent* connectionTerminated
     )
   : Thread (connectionTerminated, par),
     socket (cs), 
     repository (repo), 
     universal_object_id (objId)
  {
    assert (repo);
    assert (socket);
  }

  RConnectedSocket* socket;
  void* repository;
private:
  typedef Logger<RConnection> log;
};

#endif