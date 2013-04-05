// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REPOSITORY_H_
#define CONCURRO_REPOSITORY_H_

#include "RMutex.h"
#include "SNotCopyable.h"
#include "SException.h"
#include "SShutdown.h"
//#include "REvent.h"
//#include "StateMap.h"
#include "Logging.h"
#include <algorithm>
#include <utility>
#include <map>
#include <list>
#include <unordered_map>
#include <assert.h>

/// The Par contains invalid parameters.
class InvalidObjectParameters : public SException
{
public:
  InvalidObjectParameters ()
	 : SException 
	 ("Invalid parameters for repository object creation")
  {}
};

class AbstractRepositoryBase 
{
public:
  virtual ~AbstractRepositoryBase() {}
};

/// It is used for object creation as an argument to
/// Par::create_derivation
struct ObjectCreationInfo
{
  AbstractRepositoryBase* repository;
  std::string objectId;
};



/**
 * A RepositoryBase contain not-specialized methods of
 * Repository, i.e., methods not dependent on the ObjMap.
 */
template<
  class Obj, class Par, 
  template<class...> class ObjMap, 
  class ObjId
>
class RepositoryBase : public AbstractRepositoryBase
{
public:

  /// No object with such id exists.
  class NoSuchId : public SException
  {
  public:
	 NoSuchId (ObjId the_id) 
		: SException (SFORMAT("No object with id [" 
									 << the_id 
									 << "] exists")),
		id (the_id) {}

	 ~NoSuchId () throw () {}

	 const ObjId id;
  };

  //! It can be raised, for example, when the repository
  //! is based on Par when calculating the new Id (i.e.,
  //! unordered_map).
  class IdIsAlreadyUsed : public SException
  {
  public:
	 IdIsAlreadyUsed (const ObjId& the_id) 
		: SException (SFORMAT("The object id [" 
			 << the_id << "] is used already.")),
		  id (the_id)	 {}

	 ~IdIsAlreadyUsed () throw () {}

	 const ObjId id;
  };

  RepositoryBase (const std::string& repo_name) 
	 : objectsM (SFORMAT(repo_name << ".objectsM")) {}

  virtual ~RepositoryBase ();

  //! Create a new object in the repository by parameters
  //! Par.
  //! \exception InvalidObjectParameters something is
  //! wrong with param. 
  virtual Obj* create_object (const Par& param);

  /// Delete obj from the repository. freeMemory means
  /// to call the object desctructor after it.
  virtual void delete_object(Obj* obj, bool freeMemory);

  /// Delete the object with id.
  /// \param freeMemory call delete on the object itself
  ///        (destructor). false means only remove a registration in the
  ///        repository.
  /// \exception NoSuchId
  virtual void delete_object_by_id (ObjId id, bool freeMemory);

  virtual Obj* get_object_by_id (ObjId id) const;

  /// Replace the old object by new one (and create the new).
  /// The old object is deleted.
  /// \exception InvalidObjectParameters something is wrong with param. 
  /// \exception NoSuchId
  virtual Obj* replace_object 
	 (ObjId id, const Par& param, bool freeMemory);

  /// return ids of objects selected by a predicate
  template<class Out, class Pred>
  Out get_object_ids_by_pred (Out res, Pred p);

  /// return ids of objects selected by  an UniversalState
  template<class Out, class State>
  Out get_object_ids_by_state
    (Out res, const State& state);

  template<class Op>
  void for_each (Op& f);

  template<class Op>
  void for_each (Op& f) const;

  class Destroy 
    : public std::unary_function<int, void>
  {
  public:
    Destroy (RepositoryBase& _repo) : repo (_repo) {}
    void operator () (int objId)
    {
      repo.delete_object_by_id (objId, true);
    }
  protected:
    RepositoryBase& repo;
  };

protected:
  typedef Logger<RepositoryBase> log;

  //ObjMap<Obj*>* objects;
  RMutex objectsM;

  /// Calculate an id for a new object, possible based on Par
  /// (depending of the Repository type)
  virtual ObjId get_object_id 
	 (const ObjectCreationInfo& oi,
	  const Par&) = 0;

  /// Insert new object into objects
  virtual void insert_object (ObjId, Obj*) = 0;
};

// TODO separate read and write lock

/**
 * This is implementation of Repository for vector-like
 * objects. 
 */
template< 
  class Obj, class Par, 
  template<class...> class ObjMap, 
  class ObjId 
>
class Repository 
  : public RepositoryBase<Obj, Par, ObjMap, ObjId>,
  public SNotCopyable
{
public:
  typedef RepositoryBase<Obj, Par, ObjMap, ObjId> Parent;
  using Parent::NoSuchId;
  using Parent::IdIsAlreadyUsed;

  /// Create the repo. initial_value means initial size
  /// for vector and size for hash tables.
  Repository(const std::string& repository_name,
				size_t initial_capacity)
	: Parent (repository_name) ,
	 objects(new ObjMap<Obj*> (initial_capacity))
  {}

  ~Repository() { delete objects; }

protected:
  /// This specialization takes the first unused (numeric)
  /// id and ignores Par
  ObjId get_object_id (const ObjectCreationInfo&,
		       const Par&)
  {
	 RLOCK(this->objectsM);

	 // FIXME ! change to stack
	 for (ObjId id = 1; id < this->objects->size (); id++)
	 {
		if (!(*this->objects)[id]) return id;
	 }

	 if (this->objects->size () 
		  == this->objects->capacity ())
		this->objects->reserve 
		  (this->objects->size () 
			+ this->objects->size () * 0.2);
	 // TODO check the stepping

	 this->objects->push_back (0);
	 return this->objects->size () - 1;
  }

  void insert_object (ObjId id, Obj* obj)
  {
	 RLOCK(this->objectsM);
	 objects->at(id) = obj;
  }
protected:
  ObjMap<Obj*>* objects;
};

/**
 * This is a specialization of Repository for
 * std::unordered_map.
 */
template<class Obj, class Par, class ObjId>
class Repository<Obj, Par, std::unordered_map, ObjId>
: public RepositoryBase<Obj, Par, std::unordered_map, ObjId>,
  public SNotCopyable
{
public:
  typedef RepositoryBase<Obj, Par, std::unordered_map,
	 ObjId> Parent;
  using Parent::NoSuchId;
  using Parent::IdIsAlreadyUsed;

  typedef std::unordered_map<ObjId, Obj*> ObjMap;

  /// Create the repo. initial_value means initial size
  /// for vector and size for hash tables.
  Repository(const std::string& repository_name, 
				 size_t initial_value)
	 : Parent (repository_name),
	 objects(new ObjMap (initial_value)) 
	{
	  // id 0 is not used for real objects
	  objects->insert
		 (std::pair<ObjId, Obj*>(0, (Obj*)0));
   }

  ~Repository() { delete objects; }

protected:
  std::unordered_map<ObjId, Obj*>* objects;

  /// This specialization takes the key value from pars.
  ObjId get_object_id (const ObjectCreationInfo& oi,
		       const Par& param)
  {
	 RLOCK(this->objectsM);

	 ObjId id = param.get_id(oi);

	 if (this->objects->find(id) != this->objects->end()) {
		throw typename Parent::IdIsAlreadyUsed (id);
	 }
	 
	 return id;
  }

  void insert_object (ObjId id, Obj* obj)
  {
	 RLOCK(this->objectsM);
	 // will be add new element if id doesn't exists
	 (*this->objects)[id] = obj;
  }
};

/**
 * This is a specialization of Repository for std::map.
 */
template<class Obj, class Par, class ObjId>
class Repository<Obj, Par, std::map, ObjId>
  : public RepositoryBase<Obj, Par, std::map, ObjId>,
  public SNotCopyable
{
public:
  typedef RepositoryBase<Obj, Par, std::map,
	 ObjId> Parent;
  using Parent::NoSuchId;
  using Parent::IdIsAlreadyUsed;

  typedef std::map<ObjId, Obj*> ObjMap;
  /// Create the repo. initial_value means initial size
  /// for vector and size for hash tables.
  Repository 
	 (const std::string& repository_name, size_t initial_value)
	 : Parent (repository_name)
  {
	 this->objects = new ObjMap ();
  }
protected:
  /// This specialization takes the key value from pars.
  ObjId get_object_id 
	 (const ObjectCreationInfo& oi,
	  const Par& param)
  {
	 RLOCK(this->objectsM);

	 ObjId id = param.get_id ();

	 if (this->objects->find(id) != this->objects->end()) {
		throw typename Parent::IdIsAlreadyUsed (id);
	 }
	 
	 return id;
  }

  void insert_object (ObjId id, Obj* obj)
  {
	 RLOCK(this->objectsM);
	 // will be add new element if id doesn't exists
	 (*this->objects)[id] = obj;
  }
};

template<
  class Obj, class Par, 
  template<class...> class ObjMap, 
  class ObjId, class ObjMapValue>
class Destructor 
  : public std::unary_function<ObjMapValue, void>
{};

template<
  class Obj, class Par, 
  template<class...> class ObjMap, 
  class ObjId
>
class Destructor<Obj, Par, ObjMap, ObjId, Obj*> 
  : public std::unary_function<Obj*, void>
{
public:
  Destructor 
	 (RepositoryBase<Obj, Par, ObjMap, ObjId>* _repo)
    : repo (_repo)
  {}

  void operator () (Obj* obj)
  { 
    if (obj) 
      repo->delete_object (obj, true); 
  }

protected:
  RepositoryBase<Obj, Par, ObjMap, ObjId>* repo;
};

template<
  class Obj, class Par, 
  template<class...> class ObjMap, 
  class ObjId
>
class Destructor<
  Obj, Par, ObjMap, ObjId, std::pair<const ObjId, Obj*>
> 
: public std::unary_function
   <std::pair<const ObjId, Obj*>, void>
{
public:
  Destructor 
	 (RepositoryBase<Obj, Par, ObjMap, ObjId>* _repo)
    : repo (_repo)
  {}

  void operator() 
	 (const std::pair<const ObjId, Obj*>& pair)
  { 
    if (pair.second) 
      repo->delete_object (pair.second, true); 
  }

protected:
  RepositoryBase<Obj, Par, ObjMap, ObjId>* repo;
};


/*=====================================*/
/*========== SparkRepository ==========*/
/*=====================================*/

DEFINE_EXCEPTION(
  SeveralObjects, 
  "The param leads to several objects creation, "
  "you should use create_several_objects."
);

/**
 * A repository which replase create_object with
 * create_several_object function.  
 */
template<
  class Obj, class Par, 
  template<class...> class ObjMap, class ObjId,
  template<class...> class List = std::list
>
class SparkRepository
  : public Repository<Obj, Par, ObjMap, ObjId>
{
public:
  typedef Repository<Obj, Par, ObjMap, ObjId> Parent;

  SparkRepository
    (const std::string& repo_name, size_t init_capacity)
 : Repository<Obj, Par, ObjMap, ObjId>
      (repo_name, init_capacity) {}

  Obj* create_object (const Par& param);

  //! Some kind of params cause creation more than one
  //object.
  virtual List<Obj*> create_several_objects(Par& param);
private:
typedef Logger<
  SparkRepository<Obj, Par, ObjMap, ObjId,List>> log;
};


/*=====================================*/
/*========= helper templates ==========*/
/*=====================================*/

/**
 * A repository parameter general template.
 */
template<class Par, class Object>
class GeneralizedPar
{
public:
  virtual Object* create_derivation
    (const ObjectCreationInfo& oi) const;

  virtual Object* transform_object
    (const Object*) const
  { THROW_NOT_IMPLEMENTED; }

};

/**
 * A repository member with default universal id
 * implementation. 
 */
class StdIdMember
{
public:
  const std::string universal_object_id;

  StdIdMember(const std::string& id)
    : universal_object_id(id) {}

  std::string universal_id() const
  { return universal_object_id; }
};

#endif


