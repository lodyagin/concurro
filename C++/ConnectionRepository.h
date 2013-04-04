#ifndef CONCURRO_CONNECTIONREPOSITORY_H_
#define CONCURRO_CONNECTIONREPOSITORY_H_

#include "Repository.h"
#include "AbstractConnection.h"
#include "ThreadRepository.h"
#include <list>

template<class Connection, class Map, class Id>
class ConnectionRepository 
//
#if 0
 : public Repository<
     AbstractConnection, 
     typename AbstractConnection::Par, 
     Map, Id
   >
#else
  : public ThreadRepository<Connection, Map, Id>
#endif
{
public:
#if 0
  typedef Repository<
     AbstractConnection, 
     typename AbstractConnection::Par, 
     Map, Id
	 > Parent;
#else
  typedef ThreadRepository<Connection, Map, Id> Parent;
#endif
  typedef typename Connection::Par Par;
  typedef typename Parent::NoSuchId NoSuchId;
  
  ConnectionRepository
	 (const std::string name, size_t reserved)
	 : Parent(name, reserved) {}

  /* overrided virtuals */
  Connection* create_object(const Par&);

  void delete_object(Connection* obj, bool freeMemory);

  Connection* replace_object
	 (Id, const Par&, bool freeMemory);

  
  /* new methods */
  /// Destroy all connections in the state `closed'.
  void destroy_terminated_connections ();
};

template<class Connection, class Map, class Id>
Connection* ConnectionRepository<Connection, Map, Id>
//
::create_object(const Par& p)
{
  Connection* con = Parent::create_object(p);
  con->ask_open();
  return con;
}

template<class Connection, class Map, class Id>
void ConnectionRepository<Connection, Map, Id>
//
::delete_object(Connection* obj, bool freeMemory)
{
  // it is here but not in the Connection
  // desctructor because Repository allows delete_object
  // without freeing memory.
  obj->ask_close();

  // TODO wait closedState
  Parent::delete_object(obj, freeMemory);
}

template<class Connection, class Map, class Id>
Connection* ConnectionRepository<Connection, Map, Id>
//
::replace_object(Id id, const Par& par, bool freeMemory)
{
  RLOCK(this->objectsM);

  {
	 Connection* obj = 0;
	 try {
		obj = this->objects->at (id);
	 }
	 catch (const std::out_of_range&) {
		THROW_EXCEPTION(NoSuchId, id);
	 }

	 if (!obj) THROW_PROGRAM_ERROR;

	 obj->ask_close();
  }

  // TODO wait closedState
  Connection* new_obj = dynamic_cast<Connection*>
	 (Parent::replace_object(id, par, freeMemory));
  new_obj->ask_open();
  return new_obj;
}

template<class Connection, class Map, class Id>
void ConnectionRepository<Connection, Map, Id>
//
::destroy_terminated_connections ()
{
  // Found and destroy closed connections
  std::list<Id> terminated;
  this->get_object_ids_by_state
	 (std::back_inserter (terminated),
	  Connection::closedState
		);
  std::for_each 
	 (terminated.begin (), terminated.end (),
	  Parent::Destroy (*this));
}

#endif
