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
template<class Obj, class Par, class ObjMap, class ObjId>
class RepositoryBase : public AbstractRepositoryBase
{
public:

  /// No object with such id exists.
  class NoSuchId : public SException
  {
  public:
	 NoSuchId (ObjId the_id) 
		: id (the_id), 
		SException (SFORMAT("No object with id [" << the_id 
								  << "] exists"))
	 {}
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
		: id (the_id), 
		SException (SFORMAT("The object id [" << the_id << "] is used already."))
	 {}
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

  ObjMap* objects;
  RMutex objectsM;

  /// Calculate an id for a new object, possible based on Par
  /// (depending of the Repository type)
  virtual ObjId get_object_id (const Par&) = 0;

  /// Insert new object into objects
  virtual void insert_object (ObjId, Obj*) = 0;
};

// TODO separate read and write lock

/**
 * This is implementation of Repository for vector-like
 * objects. 
 */
template< class Obj, class Par, class ObjMap, class ObjId >
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
  Repository 
	 (const std::string& repository_name, size_t initial_value)
	 : Parent (repository_name)
  {
	 this->objects = new ObjMap (initial_value);
	 this->objects->push_back (0); // id 0 is not used for
											 // real objects
  }

protected:
  /// This specialization takes the first unused (numeric)
  /// id and ignores Par
  ObjId get_object_id (const Par&)
  {
	 RLOCK(this->objectsM);

	 // FIXME ! change to stack
	 for (ObjId id = 1; id < this->objects->size (); id++)
	 {
		if (!(*this->objects)[id]) return id;
	 }

	 if (this->objects->size () == this->objects->capacity ())
		this->objects->reserve (this->objects->size () 
										+ this->objects->size () * 0.2);
	 // TODO check the stepping

	 this->objects->push_back (0);
	 return this->objects->size () - 1;
  }

  void insert_object (ObjId id, Obj* obj)
  {
	 RLOCK(this->objectsM);
	 this->objects->at(id) = obj;
  }
};

/**
 * This is a specialization of Repository for
 * std::unordered_map.
 */
template<class Obj, class Par, class ObjId>
class Repository<Obj, Par, std::unordered_map<ObjId, Obj*>, ObjId>
  : public RepositoryBase<Obj, Par, std::unordered_map<ObjId, Obj*>, ObjId>,
  public SNotCopyable
{
public:
  typedef RepositoryBase<Obj, Par, std::unordered_map<ObjId, Obj*>,
	 ObjId> Parent;
  using Parent::NoSuchId;
  using Parent::IdIsAlreadyUsed;

  typedef std::unordered_map<ObjId, Obj*> ObjMap;
  /// Create the repo. initial_value means initial size
  /// for vector and size for hash tables.
  Repository 
	 (const std::string& repository_name, size_t initial_value)
	 : Parent (repository_name)
  {
	 this->objects = new ObjMap (initial_value);
  }
protected:
  /// This specialization takes the key value from pars.
  ObjId get_object_id (const Par& param)
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

/**
 * This is a specialization of Repository for std::map.
 */
template<class Obj, class Par, class ObjId>
class Repository<Obj, Par, std::map<ObjId, Obj*>, ObjId>
  : public RepositoryBase<Obj, Par, std::map<ObjId, Obj*>, ObjId>,
  public SNotCopyable
{
public:
  typedef RepositoryBase<Obj, Par, std::map<ObjId, Obj*>,
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
  ObjId get_object_id (const Par& param)
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

template<class Obj, class Par, class ObjMap, 
  class ObjId, class ObjMapValue>
class Destructor 
  : public std::unary_function<ObjMapValue, void>
{};

template<class Obj, class Par, class ObjMap, class ObjId>
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

template<class Obj, class Par, class ObjMap, class ObjId>
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

#if 0
/**
 * A repository parameter general template.
 */
template<class Object>
class GeneralizedPar
{
public:
  virtual Object* create_derivation
    (const ObjectCreationInfo& oi) const
  {
	 return new Object(oi, *this);
  }

  virtual Object* transform_object
    (const Object*) const
  { THROW_NOT_IMPLEMENTED; }

};
#endif

#include "Repository.hpp"

#endif


