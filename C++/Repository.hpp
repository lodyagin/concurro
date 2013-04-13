// -*-coding: mule-utf-8-unix; fill-column: 58 -*-

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REPOSITORY_HPP_
#define CONCURRO_REPOSITORY_HPP_

#include "Repository.h"
#include <set>

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

template<
  class Obj, 
  class Par, 
  template<class...> class ObjMap, 
  class ObjId 
>
RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::~RepositoryBase ()
{
  std::for_each
    (objects->begin (), 
     objects->end (),
     Destructor<Obj, Par, ObjMap, ObjId, 
	  typename RepositoryMapType<Obj, ObjId, ObjMap>
	    ::Map::value_type> (this)
     );
  delete objects;
}

template<
  class Obj, 
  class Par, 
  template<class...> class ObjMap, 
  class ObjId
>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::create_object (const Par& param)
{
  /*if (SThread::current ().is_stop_requested ())
       ::xShuttingDown 
        (L"Stop request from the owner thread.");*/
  Obj* obj = 0;
  // <NB> cinfo.objectId is empty at the first call to param
  ObjectCreationInfo cinfo;
  cinfo.repository = this;
  { 
    RLOCK (objectsM);

    const ObjId objId = get_object_id(cinfo, param);
    toString (objId, cinfo.objectId);

	 // dynamic cast for use with inherited parameters
	 // <NB> this must be called inside the lock to allow
	 // query repo data structures from create_derivation
    obj = dynamic_cast<Obj*>
		(param.create_derivation (cinfo));

    SCHECK (obj);
    insert_object (objId, obj);
  }
  LOG_TRACE(log, "Object " << *obj << " is created.");

  if (cinfo.objectCreated)
	 cinfo.objectCreated->set(); 
    // <NB> after inserting into repo
    // and unlocking

  return obj;
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::replace_object (ObjId id, const Par& param, bool freeMemory)
{
  RLOCK(objectsM);

  Obj* obj = 0;
  try {
	 obj = objects->at (id);
  }
  catch (const std::out_of_range&) {
	 THROW_EXCEPTION(NoSuchId, id);
  }

  if (!obj) THROW_PROGRAM_ERROR;
  //obj->freeze();

  (*objects)[id] = dynamic_cast<Obj*>
	 (param.transform_object (obj));

  SCHECK ((*objects)[id]);

  if ((*objects)[id] == obj)
    return obj; // no transformation

  if (freeMemory)
    delete obj;

  return (*objects)[id];
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::delete_object (Obj* obj, bool freeMemory)
{
  assert (obj);
  const ObjId objId = fromString<ObjId> 
	 (obj->universal_id());

  delete_object_by_id (objId, freeMemory);
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::delete_object_by_id (ObjId id, bool freeMemory)
{
  Obj* ptr = 0;
  {
    RLOCK(objectsM);

	 try {
		Obj*& r = objects->at (id);
		ptr = r;
		if (r == 0) THROW_PROGRAM_ERROR;
		r = 0;
	 }
	 catch (const std::out_of_range&) {
		THROW_EXCEPTION(NoSuchId, id);
	 }
  }
  //ptr->freeze();
  if (freeMemory) delete ptr;
}

template <class Obj, class Par, template<class...> class ObjMap, class ObjId>
Obj* RepositoryBase <Obj, Par, ObjMap, ObjId>
//
::get_object_by_id (ObjId id) const
{
  try { 
    RLOCK(objectsM);
	 return objects->at (id);
  }
  catch (const std::out_of_range&)
  {
	 THROW_EXCEPTION(NoSuchId, id);
  }
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
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

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
template<class Out, class State>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_state (Out res, const State& state)
{
  return get_object_ids_by_pred<Out, StateMatch<Obj,
  State>> 
	 (res, StateMatch<Obj, State> (state));
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
template<class Op>
void RepositoryBase<Obj, Par, ObjMap, ObjId>::for_each (Op& f)
{
  RLOCK(objectsM);

  for (ObjId i = 0; i < objects->size (); i++)
    if ((*objects)[i])
      f (*(*objects)[i]);
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
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

/*=====================================*/
/*========== SparkRepository ==========*/
/*=====================================*/

template<
  class Obj, class Par, template<class...> class ObjMap, class ObjId,
  template<class...> class List
>
Obj* SparkRepository<Obj, Par, ObjMap, ObjId, List>
//
::create_object (const Par& param)
{
  //THROW_EXCEPTION(SeveralObjects, void);
  throw SeveralObjects();
}

template<
  class Obj, class Par, template<class...> class ObjMap, class ObjId,
  template<class...> class List
>
List<Obj*> SparkRepository<Obj, Par, ObjMap, ObjId, List>
//
::create_several_objects(Par& param)
{
  List<Obj*> out;
  Obj* obj = 0;

  ObjectCreationInfo cinfo;
  cinfo.repository = this;
  {
      RLOCK (this->objectsM);
      const size_t n = param.n_objects(cinfo); 
      // count objects to be created (it also can create
      // them)

      for (size_t k = 0; k < n; k++)
      { 
        cinfo.objectId.clear();
        const ObjId objId = get_object_id(cinfo, param);
        toString(objId, cinfo.objectId);

        // dynamic cast for use with inherited parameters
        obj = dynamic_cast<Obj*>
          (param.create_next_derivation (cinfo));
        SCHECK (obj);
        insert_object (objId, obj);
        LOG_TRACE(log, 
						"Object " << *obj << " is created.");
		  
		  if (cinfo.objectCreated)
			 cinfo.objectCreated->set();

        out.push_back(obj);
      }
  }
  return out;
}


/*=====================================*/
/*========= helper templates ==========*/
/*=====================================*/

#if 0
template<class Par, class Object>
Object* GeneralizedPar<Par, Object>::create_derivation
    (const ObjectCreationInfo& oi) const
{
  return new Object(oi, dynamic_cast<const Par&>(*this));
}
#endif

#endif
