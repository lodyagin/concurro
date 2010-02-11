/*
It is a connection created by the socket.
*/

#pragma once

#include "sthread.h"
#include "RConnectedSocket.h"
#include "Logging.h"

//template<class Thread>
//struct ConnectionPars;

template<class Thread>
class RConnection : public Thread
{ //TODO add states
  //friend ConnectionPars<Thread>;

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
     const typename Thread::ConstrPar& par
     )
   : Thread (par),
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
  static Logging log;
};
