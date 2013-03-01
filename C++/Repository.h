// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef REPOSITORY_H_
#define REPOSITORY_H_

#include "RMutex.h"
#include "SNotCopyable.h"
#include "SException.h"
#include "SShutdown.h"
#include "REvent.h"
#include "RThread.h"
#include "StateMap.h"
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <assert.h>

class InvalidObjectParameters : public SException
{
public:
  InvalidObjectParameters ()
    : SException 
      ("Invalid parameters for repository object creation")
  {}
};

class IdIsAlreadyUsed : public SException
{
public:
  IdIsAlreadyUsed ()
    : SException 
      ("This Id is already used in the repository")
  {}
};

/**
 * A RepositoryBase contain not-specialized methods of
 * Repository, i.e., methods not dependent on the ObjMap.
 */
template<class Obj, class Par, class ObjMap, class ObjId>
class RepositoryBase
{
public:
  RepositoryBase () {}
//  RepositoryBase (size_t initial_value);

  virtual ~RepositoryBase ();

  virtual Obj* create_object (const Par& param);

  /// Delete obj from the repository. freeMemory means
  /// to call the object desctructor after it.
  virtual void delete_object(Obj* obj, bool freeMemory);

  virtual void delete_object_by_id 
	 (ObjId id, bool freeMemory);

  virtual Obj* get_object_by_id (ObjId id) const;

  // Replace the old object by new one
  // The old object is deleted
  virtual Obj* replace_object 
	 (ObjId id, const Par& param, bool freeMemory);

  // return ids of objects selected by a predicate
  template<class Out, class Pred>
  Out get_object_ids_by_pred (Out res, Pred p);

  // return ids of objects selected by 
  // an UniversalState
  template<class Out, class State>
  Out get_object_ids_by_state
    (Out res, const State& state);

  template<class Op>
  void for_each (Op& f);

  template<class Op>
  void for_each (Op& f) const;

  // It is used for object creation
  struct ObjectCreationInfo
  {
    //const Par* info;
    RepositoryBase* repository;
	 std::string objectId;
  };

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
  ObjMap* objects;
  RMutex objectsM;

  virtual ObjId get_object_id (const Par&) = 0;
  /// Insert new object into objects
  virtual void insert_object (ObjId, Obj*) = 0;
};

// TODO separate read and write lock

/**
 * This is implementation of Repository for vector-like
 * objects. 
 */
template<
  class Obj, 
  class Par, 
  class ObjMap, 
  class ObjId/*, 
					class ObjMapValue*/
>
class Repository 
  : public RepositoryBase<Obj, Par, ObjMap, ObjId>,
  public SNotCopyable
{
public:
  /// Create the repo. initial_value means initial size
  /// for vector and size for hash tables.
  Repository (size_t initial_value)
  {
	 this->objects = new ObjMap (initial_value);
	 this->objects->push_back (0); // id 0 is not used for
											 // real objects
  }

#if 0
  //virtual ~Repository ();

  virtual Obj* create_object (const Par& param);

  /// Delete obj from the repository. freeMemory means
  /// to call the object desctructor after it.
  virtual void delete_object(Obj* obj, bool freeMemory);

  virtual void delete_object_by_id 
	 (ObjId id, bool freeMemory);

  virtual Obj* get_object_by_id (ObjId id) const;

  // Replace the old object by new one
  // The old object is deleted
  virtual Obj* replace_object 
    (ObjId id, 
     const Par& param,
     bool freeMemory
     );

  // return ids of objects selected by a predicate
  template<class Out, class Pred>
  Out get_object_ids_by_pred (Out res, Pred p);

  // return ids of objects selected by 
  // an UniversalState
  template<class Out, class State>
  Out get_object_ids_by_state
    (Out res, const State& state);

  template<class Op>
  void for_each (Op& f);

  template<class Op>
  void for_each (Op& f) const;

  // It is used for object creation
  struct ObjectCreationInfo
  {
    //const Par* info;
    Repository* repository;
	 std::string objectId;
  };

  class Destroy 
    : public std::unary_function<int, void>
  {
  public:
    Destroy (Repository& _repo) : repo (_repo) {}
    void operator () (int objId)
    {
      repo.delete_object_by_id (objId, true);
    }
  protected:
    Repository& repo;
  };

protected:
  ObjId get_object_id (const Par&);

//  enum {startMapSize = 10, mapSizeStep = 10};

  //SSemaphore semaphore;
#endif

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
 * This is implementation of Repository for hash-like
 * objects. 
 */
template<class Obj, class Par, class ObjId>
class Repository<Obj, Par, std::unordered_map<ObjId, Obj*>, ObjId>
  : public RepositoryBase<Obj, Par, std::unordered_map<ObjId, Obj*>, ObjId>,
  public SNotCopyable
{
public:
  typedef std::unordered_map<ObjId, Obj*> ObjMap;
  /// Create the repo. initial_value means initial size
  /// for vector and size for hash tables.
  Repository (size_t initial_value)
  {
	 this->objects = new ObjMap (initial_value);
  }
protected:
  /// This specialization takes the key value from pars.
  ObjId get_object_id (const Par& param)
  {
	 RLOCK(this->objectsM);

	 ObjId id = param.get_id ();

	 if (this->objects->find(id) != this->objects->end())
		throw IdIsAlreadyUsed ();
	 
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
class Destructor<Obj, Par, ObjMap, ObjId, std::pair<const ObjId, Obj*>> 
  : public std::unary_function<std::pair<const ObjId, Obj*>, void>
{
public:
  Destructor (RepositoryBase<Obj, Par, ObjMap, ObjId>* _repo)
    : repo (_repo)
  {}

  void operator () (const std::pair<const ObjId, Obj*>& pair)
  { 
    if (pair.second) 
      repo->delete_object (pair.second, true); 
  }

protected:
  RepositoryBase<Obj, Par, ObjMap, ObjId>* repo;
};

template<
  class Obj, 
  class Par, 
  class ObjMap, 
  class ObjId 
>
RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::~RepositoryBase ()
{
  std::for_each
    (objects->begin (), 
     objects->end (),
     Destructor<Obj, Par, ObjMap, ObjId, typename ObjMap::value_type> (this)
     );
  delete objects;
}

template<
  class Obj, 
  class Par, 
  class ObjMap, 
  class ObjId
>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::create_object (const Par& param)
{
  /*if (SThread::current ().is_stop_requested ())
       ::xShuttingDown 
        (L"Stop request from the owner thread.");*/

  { 
    RLOCK (objectsM);

    const ObjId objId = get_object_id(param);

    std::string uniId;
    toString (objId, uniId);
    const ObjectCreationInfo cinfo = { this, uniId };

    Obj* obj = param.create_derivation (cinfo);
    SCHECK (obj);
    insert_object (objId, obj);
    return obj;
  }
}

template<class Obj, class Par, class ObjMap, class ObjId>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::replace_object (ObjId id, const Par& param, bool freeMemory)
{
  RLOCK(objectsM);

  Obj* obj = objects->at (id);
  if (!obj)
    THROW_EXCEPTION
      (SException, oss_ << L"Program error");

  (*objects)[id] = param.transform_object (obj);
  SCHECK ((*objects)[id]);

  if ((*objects)[id] == obj)
    return obj; // no transformation

  if (freeMemory)
    delete obj;

  return (*objects)[id];
}

template<class Obj, class Par, class ObjMap, class ObjId>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::delete_object (Obj* obj, bool freeMemory)
{
  assert (obj);
  const ObjId objId = fromString<ObjId> (obj->universal_object_id);

  delete_object_by_id (objId, freeMemory);
}

template<class Obj, class Par, class ObjMap, class ObjId>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::delete_object_by_id (ObjId id, bool freeMemory)
{
  Obj* ptr = 0;
  {
    RLOCK(objectsM);
    Obj* r = objects->at (id);
    ptr = r;
    if (r == 0) 
		THROW_EXCEPTION(SException, "Program error");
    r = 0;
  }
  if (freeMemory) delete ptr;
}

/*
template<
  class Obj, 
  class Par, 
  class ObjMap, 
  class ObjId, 
  class Obj*
>
void Repository<Obj, Par, ObjMap, ObjId, Obj*>
//
::delete_object_by_id (ObjId id, bool freeMemory)
{
  Obj* ptr = 0;
  {
    RLOCK(objectsM);
    typename ObjMap::reference r = objects->at (id);
    ptr = r;
    if (r == 0) 
		THROW_EXCEPTION(SException, "Program error");
    r = 0;
  }
  if (freeMemory) delete ptr;
}
*/

template <class Obj, class Par, class ObjMap, class ObjId>
Obj* RepositoryBase <Obj, Par, ObjMap, ObjId>
//
::get_object_by_id (ObjId id) const
{
  { 
    RLOCK(objectsM);

    /*if (id < 1 || id >= objects->size ())
      return 0;
		else*/
      return objects->at (id);
  }
}

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Out, class Pred>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_pred (Out res, Pred p)
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i] && p (*(*objects)[i]))
      *res++ = i;
  return res;
}

template<class Obj, class State>
class StateMatch 
  : public std::unary_function<const Obj&, bool>
{
public:
  StateMatch (const State& _state) : state (_state) {}
  bool operator () (const Obj& obj) const
  { 
    return State::state_is (obj, state);
  }
protected:
  State state;
};

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Out, class State>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_state (Out res, const State& state)
{
  return get_object_ids_by_pred<Out, StateMatch<Obj,
  State>> 
	 (res, StateMatch<Obj, State> (state));
}

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Op>
void RepositoryBase<Obj, Par, ObjMap, ObjId>::for_each (Op& f)
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i])
      f (*(*objects)[i]);
}

template<class Obj, class Par, class ObjMap, class ObjId>
template<class Op>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::for_each (Op& f) const
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i])
      f (*(*objects)[i]);
}

#endif


