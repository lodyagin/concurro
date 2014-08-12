/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REPOSITORY_H_
#define CONCURRO_REPOSITORY_H_

#include <string>
#include <algorithm>
#include <utility>
#include <list>
#include <map>
#include <unordered_map>
#include <assert.h>
#include <atomic>
#include <functional>
#include "types/exception.h"
#include "Logging.h"
#include "RMutex.h"
#include "SNotCopyable.h"
#include "Event.h"
#include "HasStringView.h"

namespace curr {

/**
 * @addtogroup exceptions
 * @{
 */

//! Exception: repository operation
struct RepositoryException : virtual std::exception {};

//! Exception: invalid parameters were defined for
//! creation of a repository object.
struct InvalidObjectParameters : RepositoryException {};
/*
{
public:
  InvalidObjectParameters()
    : curr::SException(
      "Invalid parameters were defined for creation "
      "of a repository object") {}
};
*/

//! Exception: No object with such id exists.
struct NoSuchId : RepositoryException {};

//! It can be raised, for example, when the repository
//! is based on Par when calculating the new Id (i.e.,
//! unordered_map).
struct IdIsAlreadyUsed : RepositoryException {};

//! Exception: Par::transform_object is not implemented
struct TransformObjectIsNotImplemented
  : RepositoryException {};

//! @}
 

/**
 * @defgroup repositories
 * A repository is just a collection with a concurrency.
 * @{
 */

//! Tune a logging
struct RepositoryLogParams
{
  RepositoryLogParams()
  : get_object_by_id(true) {}

  bool get_object_by_id;
};

/**
 * An abstract base for all repositories. Contains
 * get_traits() for query type infos and methods not
 * depending on template parameters unlike those declared
 * in descendants.
 */
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

  //! Return the number of (filled) elements
  virtual size_t size() const = 0;

  //! Return parameterized type infos
  virtual Traits get_traits() const = 0;

  RepositoryLogParams& log_params() const
  {
    return log_params_;
  }

protected:
  mutable RepositoryLogParams log_params_;
};

/** It defines an interface for RepositoryBase but
 * is abstracted from a Container template parameter.
 */
template<class Obj, class Par, class ObjId>
  class RepositoryInterface 
  : public AbstractRepositoryBase
{
public:

  typedef ObjId ObjIdType;
  static const Traits traits;

#if 0
  //! Exception: No object with such id exists.
  class NoSuchId : public SException
  {
  public:
    NoSuchId (const ObjId& the_id) 
      : SException (
          sformat(
            "No object with id [",
            the_id,
            "] exists in ",
            type<RepositoryInterface>::name()
          )
        ),
      id (the_id) {}

    ~NoSuchId () throw () {}

    const ObjId id;
  };
#else
  //! Exception: curr::NoSuchId descendant for specific
  //! repository. 
  struct NoSuchId : curr::NoSuchId {};
#endif

  //! Exception: curr::IdIsAlreadyUsed descendant for
  //! specific repository. 
  struct IdIsAlreadyUsed : curr::IdIsAlreadyUsed {};

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
    (const ObjId& id, bool freeMemory) = 0;

  //! \exception NoSuchId
  virtual Obj* get_object_by_id (const ObjId& id) const= 0;

  //! Replace the old object by new one (and create the
  //! new). The old object is deleted.
  //! \exception InvalidObjectParameters something is
  //!   wrong with param.
  //! \exception NoSuchId
  virtual Obj* replace_object 
   (const ObjId& id, const Par& param, bool freeMemory)= 0;

  //! Create a possible id of a new object. It is stable
  //! when Par has get_id method (for repositories based
  //! on map, hash etc) or when called from other
  //! Repository methods (when objectsM is accquired). In
  //! other cases somebody can get the same id for
  //! different objects by this function.
  virtual ObjId allocate_new_object_id (const Par&) = 0;

  //! It calls f(Obj&) for each object in the repository
  virtual void for_each
    (std::function<void(Obj&)> f) = 0;

  virtual void for_each
    (std::function<void(const Obj&)> f) const = 0;

  //! The version of create_object() with internal dynamic
  //! cast to the type T*.
  template<class T>
  T* create(const Par& param)
  {
    return dynamic_cast<T*>(create_object(param));
  }

  //! The version of get_object_by_id() with internal
  //! dynamic cast to the type T*.
  template<class T>
  T* get_by_id(const ObjId& id)
  {
    return dynamic_cast<T*>(get_object_by_id(id));
  }
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

template<class Obj, class ObjId>
  class RepositoryMapType<Obj, ObjId, std::unordered_map>
{
public:
  typedef std::unordered_map<ObjId, Obj*> Map;
};

template<class Obj, class ObjId>
  class RepositoryMapType<Obj, ObjId, std::map>
{
public:
  typedef std::map<ObjId, Obj*> Map;
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
  using typename Parent::NoSuchId;
  using typename Parent::IdIsAlreadyUsed; 
  
  RepositoryBase
    (const std::string& repo_name) 
    : objectsM (SFORMAT(repo_name << ".objectsM")),
    objects(0)
    {}

  virtual ~RepositoryBase ();

  Obj* create_object (const Par& param);
  void delete_object(Obj* obj, bool freeMemory);
  void delete_object_by_id(const ObjId& id, bool freeMemory);

  //! \exception NoSuchId
  Obj* get_object_by_id (const ObjId& id) const;

  Obj* replace_object 
    (const ObjId& id, const Par& param, bool freeMemory);

  //! return ids of objects selected by a predicate
  template<class Out, class Pred>
  Out get_object_ids_by_pred (Out res, Pred p) const;

  //! return ids of objects selected by  an UniversalState
  //TODO add event on change possible result
  template<class Out, class State>
  Out get_object_ids_by_state
    (Out res, const State& state) const;

  void for_each
    (std::function<void(Obj&)> f) override;

  void for_each
    (std::function<void(const Obj&)> f) const override;

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

  ObjId allocate_new_object_id(const Par& par) override
  {
    ObjectCreationInfo cinfo;
    cinfo.repository = this;
    return allocate_new_object_id_internal(cinfo, par);
  }

protected:
  //! Return the element itself or pair::second
  class Value
  {
  public:
    Value(Obj* t0) : t(t0) {}
    Value(const std::pair<ObjId,Obj*>& p) : t(p.second) {}
    operator Obj*() { return t; }
    Obj* operator->() { return t; }

  protected:
    Obj* t;
  };

  RMutex objectsM;
  typename RepositoryMapType<Obj, ObjId, ObjMap>
    ::Map* objects;

  virtual ObjId allocate_new_object_id_internal
    (ObjectCreationInfo& oi, const Par&) = 0;

  //! Insert new object into objects
  virtual void insert_object (const ObjId&, Obj*) = 0;
  //! Free an object cell. <NB> it is empty in
  //! RepositoryBase to allow (dummy) calls from
  //! destructor.
  virtual void delete_object_id (const ObjId&) {}

private:
  typedef Logger<RepositoryBase> log;
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

  typedef Obj Object;
  typedef Par Parameter;
  typedef ObjId ObjectId;

  using typename Parent::NoSuchId;
  using typename Parent::IdIsAlreadyUsed;

  //! Create the repo. initial_value means initial size
  //! for vector and size for hash tables.
  Repository(const std::string& repository_name,
           size_t initial_capacity)
  : Parent (repository_name),
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
  ObjId allocate_new_object_id_internal 
    (ObjectCreationInfo&, const Par&) override
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

  void insert_object (const ObjId& id, Obj* obj)
  {
    {
      RLOCK(this->objectsM);
      this->objects->at(id) = obj;
    }
    obj_count++;
  }

  void delete_object_id (const ObjId& id)
  {
    {
      RLOCK(this->objectsM);
      Obj*& obj = this->objects->at(id);
      if (obj) {
        obj = 0;
        obj_count--;
      }
    }
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
  : public RepositoryBase
      <Obj, Par, std::unordered_map, ObjId>,
    public SNotCopyable
{
public:
  typedef RepositoryBase<Obj, Par, std::unordered_map,
    ObjId> Parent;
  using typename Parent::NoSuchId;
  using typename Parent::IdIsAlreadyUsed;

  typedef std::unordered_map<ObjId, Obj*> ObjMap;

  //! Create the repo. initial_value means initial size
  //! for vector and size for hash tables.
  Repository(const std::string& repository_name, 
           size_t initial_value)
    : Parent (repository_name)
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
  ObjId allocate_new_object_id_internal(
    ObjectCreationInfo& oi, 
    const Par& param
  );

  void insert_object (const ObjId& id, Obj* obj)
  {
    RLOCK(this->objectsM);
    // will be add new element if id doesn't exists
    (*this->objects)[id] = obj;
  }

  void delete_object_id(const ObjId& id)
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
  using typename Parent::NoSuchId;
  using typename Parent::IdIsAlreadyUsed;

  typedef std::map<ObjId, Obj*> ObjMap;
  //! Create the repo. initial_value means initial size
  //! for vector and size for hash tables.
  Repository 
    (const std::string& repository_name, 
     size_t initial_value)
    : Parent (repository_name)
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
  ObjId allocate_new_object_id_internal 
    (ObjectCreationInfo& oi, const Par& param)
  {
    RLOCK(this->objectsM);

    ObjId id = param.get_id (oi);

    if (this->objects->find(id) != this->objects->end()) {
      throw ::types::exception <
        typename Parent::IdIsAlreadyUsed
      > ("The object id [", id, "] is used already.");
    }
	 
    return id;
  }

  void insert_object (const ObjId& id, Obj* obj)
  {
    RLOCK(this->objectsM);
    // will be add new element if id doesn't exists
    (*this->objects)[id] = obj;
  }

  void delete_object_id(const ObjId& id)
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

//! Exception: in SparkRepository
struct SparkRepositoryException : RepositoryException {};

//! Exception: the param leads to several objects
//! creation, you should use create_several_objects.
struct SeveralObjects : SparkRepositoryException {};
#if 0
{
public:
  SeveralObjects() 
    : curr::SException(
      "The param leads to several objects creation, "
      "you should use create_several_objects.") {}
};
#endif

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
  typedef List<Obj*> list_type;
  typedef Repository<Obj, Par, ObjMap, ObjId> Parent;

  SparkRepository
    (const std::string& repo_name, size_t init_capacity)
    : Repository<Obj, Par, ObjMap, ObjId>
    (repo_name, init_capacity) {}

  Obj* create_object (const Par& param);

  //! Some kind of params cause creation more than one
  //! object.
  virtual List<Obj*> create_several_objects(Par& param);
private:
  typedef Logger<
    SparkRepository<Obj, Par, ObjMap, ObjId,List>> log;
};


/*=====================================*/
/*========= helper templates ==========*/
/*=====================================*/

#define STD_DERIVATION(Object)

/**
 * A repository member with default universal id
 * implementation. 
 */
class StdIdMember : public HasStringView
{
public:
  const std::string universal_object_id;

  StdIdMember(const std::string& id)
  : universal_object_id(id) {}

  //! A default copy constructor.
  StdIdMember(const StdIdMember&) = default;

  //! An empty virtual destructor.
  virtual ~StdIdMember() {}

  //! A default assignment operator.
  StdIdMember& operator=(const StdIdMember&) = default;

  void outString (std::ostream& out) const override
  { out << object_name(); }

  std::string universal_id() const
  { return universal_object_id; }

  //! Form an object name as a typeid : univeral_id.
  virtual std::string object_name() const
  {
    return sformat
      (::types::type<decltype(*this)>::name(), ':',
       universal_id()); 
  }
};

template<class ObjectId>
class StdIdMemberT : public StdIdMember
{
public:
  const ObjectId id;

  StdIdMemberT(const ObjectCreationInfo& oi)
    : StdIdMember(oi.objectId),
      id(fromString<ObjectId>(oi.objectId))
  {}

  ObjectId get_id() const { return id; }
};

//! The simplest form of a derivation
#define PAR_CREATE_DERIVATION(type, parent, object)     \
  struct Par : public parent::Par                       \
  {                                                     \
    object* create_derivation                           \
      (const curr::ObjectCreationInfo& oi) const        \
    { return new type(oi, *this); }                     \
  };

#define PAR_DEFAULT_MEMBERS(object)             \
  object* create_derivation                     \
  (const curr::ObjectCreationInfo& oi) const    \
  { return new object(oi, *this); }             \
                                                \
  object* transform_object                      \
  (const object*) const                         \
  {                                             \
    throw ::types::exception                    \
      <TransformObjectIsNotImplemented>         \
        (#object ":transform_object is not implemented"); \
  }

#define PAR_DEFAULT_VIRTUAL_MEMBERS(object)     \
  virtual ~Par() {}                             \
                                                \
  virtual object* create_derivation             \
  (const curr::ObjectCreationInfo& oi) const    \
  { return new object(oi, *this); }             \
                                                \
  virtual object* transform_object              \
  (const object*) const                         \
  {                                             \
    throw ::types::exception                    \
      <TransformObjectIsNotImplemented>         \
        (#object ":transform_object is not implemented"); \
  }

#define PAR_DEFAULT_ABSTRACT(object)            \
  virtual ~Par() {}                             \
                                                \
  virtual object* create_derivation             \
  (const curr::ObjectCreationInfo& oi) const=0; \
                                                \
  virtual object* transform_object              \
  (const object*) const                         \
  {                                             \
    throw ::types::exception                    \
      <TransformObjectIsNotImplemented>         \
        (#object ":transform_object is not implemented"); \
  }

#define PAR_DEFAULT_OVERRIDE(ret, object)              \
  virtual ret* create_derivation                       \
  (const curr::ObjectCreationInfo& oi) const override  \
  { return new object(oi, *this); }

#define REPO_OBJ_CONSTRUCTOR(object)         \
protected:                                   \
  object(const curr::ObjectCreationInfo& oi, \
         const typename object::Par& par);

#define REPO_OBJ_INHERITED_CONSTRUCTOR_DEF(object, \
  parent, base)                                    \
protected:                                   \
  object(const curr::ObjectCreationInfo& oi, \
         const typename base::Par& par)      \
    : parent(oi, par) {}

//! @}

}

#endif


