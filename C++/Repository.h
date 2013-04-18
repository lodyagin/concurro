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
#include "Logging.h"
#include "Event.h"
#include <algorithm>
#include <utility>
#include <list>
#include <map>
#include <unordered_map>
#include <assert.h>
#include <atomic>

DEFINE_EXCEPTION(InvalidObjectParameters,
  "Invalid parameters for repository object creation"
)

class AbstractRepositoryBase 
{
public:
  struct Traits 
  {
	 std::string obj, par, obj_id;

	 bool operator== (const Traits& t) const
	 {
		return obj == t.obj && par == t.par 
		  && obj_id == t.obj_id;
	 }
  };

  virtual ~AbstractRepositoryBase() {}
  virtual size_t size() const = 0;
  virtual Traits get_traits() const = 0;
};

/** It defines an interface for RepositoryBase but
 * is abstracted from a Container template parameter.
 */
template<class Obj, class Par, class ObjId>
class RepositoryInterface 
  : public AbstractRepositoryBase
{
public:

  static const Traits traits;

  //! No object with such id exists.
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

  //! A method for debug dynamic_cast issues
  static bool is_compatible
	 (const AbstractRepositoryBase* r)
  { 
	 assert(r);
	 return r->get_traits() == traits;
  }

  Traits get_traits() const
  { return traits; }
  
  //! Create a new object in the repository by parameters
  //! Par.
  //! \exception InvalidObjectParameters something is
  //! wrong with param. 
  virtual Obj* create_object (const Par& param) = 0;
  
  //! Delete obj from the repository. freeMemory means
  //! to call the object desctructor after it.
  virtual void delete_object
	 (Obj* obj, bool freeMemory) = 0;

  //! Delete the object with id.
  //! \param freeMemory call delete on the object itself
  //!  (destructor). false means only remove a
  //!  registration in the repository.
  //! \exception NoSuchId
  virtual void delete_object_by_id 
	 (ObjId id, bool freeMemory) = 0;

  virtual Obj* get_object_by_id (ObjId id) const = 0;

  //! Replace the old object by new one (and create the
  //! new). The old object is deleted.
  //! \exception InvalidObjectParameters something is
  //!   wrong with param.
  //! \exception NoSuchId
  virtual Obj* replace_object 
	 (ObjId id, const Par& param, bool freeMemory) = 0;
};

//! It is used for object creation as an argument to
//! Par::create_derivation
struct ObjectCreationInfo
{
  AbstractRepositoryBase* repository;
  std::string objectId;
  Event* objectCreated;

  ObjectCreationInfo()
  : repository(0), objectCreated(0) {}
};

template<
  class Obj, class ObjId,
  template<class...> class ObjMap
>
class RepositoryMapType
{
public:
  typedef ObjMap<Obj*> Map;
};

//! Tune a logging
struct RepositoryLogParams
{
  RepositoryLogParams()
  : get_object_by_id__no_such_id(true) {}

  bool get_object_by_id__no_such_id;
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
class RepositoryBase 
  : public RepositoryInterface<Obj, Par, ObjId>
{
public:
  typedef RepositoryInterface<Obj, Par, ObjId> Parent;
  typedef typename Parent::NoSuchId NoSuchId;
  typedef typename Parent::IdIsAlreadyUsed 
	 IdIsAlreadyUsed;
  typedef RepositoryLogParams LogParams;
  
  RepositoryBase
	 (const std::string& repo_name, 
	  const LogParams& log_params_ = LogParams()) 
	 : objectsM (SFORMAT(repo_name << ".objectsM")),
	 objects(0), log_params(log_params_)
	 {}

  virtual ~RepositoryBase ();

  Obj* create_object (const Par& param);
  void delete_object(Obj* obj, bool freeMemory);
  void delete_object_by_id(ObjId id, bool freeMemory);
  Obj* get_object_by_id (ObjId id) const;

  Obj* replace_object 
	 (ObjId id, const Par& param, bool freeMemory);

  //! return ids of objects selected by a predicate
  template<class Out, class Pred>
  Out get_object_ids_by_pred (Out res, Pred p);

  //! return ids of objects selected by  an UniversalState
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

  RMutex objectsM;
  typename RepositoryMapType<Obj, ObjId, ObjMap>
	 ::Map* objects;
  LogParams log_params;

  //! Calculate an id for a new object, possible based on
  //! Par (depending of the Repository type)
  virtual ObjId get_object_id 
	 (ObjectCreationInfo& oi, const Par&) = 0;

  //! Insert new object into objects
  virtual void insert_object (ObjId, Obj*) = 0;
  //! Free an object cell. <NB> it is empty in
  //! RepositoryBase to allow (dummy) calls from
  //! destructor.
  virtual void delete_object_id (ObjId) {}
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

  //! Create the repo. initial_value means initial size
  //! for vector and size for hash tables.
  Repository(const std::string& repository_name,
				 size_t initial_capacity,
				 const RepositoryLogParams& log_params_ = 
				 RepositoryLogParams())
	 : Parent (repository_name, log_params_),
	 obj_count(0)
  {
	 this->objects = new typename RepositoryMapType<Obj,
		ObjId, ObjMap>::Map (initial_capacity);
	 this->objects->push_back (0); // id 0 is not used for
											 // real objects
  }

  size_t size() const
  {
	 assert(this->objects);
	 assert(this->objects->size() > obj_count);
	 return obj_count.load();
  }

protected:
  std::atomic<unsigned int> obj_count;

  //! This specialization takes the first unused (numeric)
  //! id and ignores Par
  ObjId get_object_id 
	 (ObjectCreationInfo&, const Par&)
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
	 {
		RLOCK(this->objectsM);
		this->objects->at(id) = obj;
	 }
	 obj_count++;
  }

  void delete_object_id (ObjId id)
  {
	 {
		RLOCK(this->objectsM);
		this->objects->at(id) = 0;
	 }
	 obj_count--;
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

  //! Create the repo. initial_value means initial size
  //! for vector and size for hash tables.
  Repository(const std::string& repository_name, 
				 size_t initial_value,
				 const RepositoryLogParams& log_params_ = RepositoryLogParams())
	 : Parent (repository_name, log_params_)
  {
	 this->objects = new ObjMap (initial_value);
  }

  size_t size() const
  {
	 assert(this->objects);
	 RLOCK(this->objectsM);
	 return this->objects->size();
  }

protected:
  //! This specialization takes the key value from pars.
  ObjId get_object_id 
	 (ObjectCreationInfo& oi, const Par& param)
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

  void delete_object_id(ObjId id)
  {
	 assert(this->objects);
	 RLOCK(this->objectsM);
	 const size_t n = this->objects->erase(id);
	 assert(n == 1);
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
  //! Create the repo. initial_value means initial size
  //! for vector and size for hash tables.
  Repository 
	 (const std::string& repository_name, 
	  size_t initial_value,
		const RepositoryLogParams& log_params_ = RepositoryLogParams())
	 : Parent (repository_name, log_params_)
  {
	 this->objects = new ObjMap ();
  }

  size_t size() const
  {
	 assert(this->objects);
	 RLOCK(this->objectsM);
	 return this->objects->size();
  }

protected:
  //! This specialization takes the key value from pars.
  ObjId get_object_id 
	 (ObjectCreationInfo& oi, const Par& param)
  {
	 RLOCK(this->objectsM);

	 ObjId id = param.get_id (oi);

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

  void delete_object_id(ObjId id)
  {
	 assert(this->objects);
	 RLOCK(this->objectsM);
	 const size_t n = this->objects->erase(id);
	 assert(n == 1);
  }
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

#if 0
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
#else
#define STD_DERIVATION(Object)
#endif

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

//! The simplest form of a derivation
#define PAR_CREATE_DERIVATION(type, parent, object) \
struct Par : public parent::Par \
{ \
  object* create_derivation \
	 (const ObjectCreationInfo& oi) const \
  { return new type(oi, *this); } \
};

#define PAR_DEFAULT_MEMBERS(object) \
  object* create_derivation \
	 (const ObjectCreationInfo& oi) const \
  { return new object(oi, *this); } \
  \
  object* transform_object \
    (const object*) const \
  { THROW_NOT_IMPLEMENTED; }


#endif


