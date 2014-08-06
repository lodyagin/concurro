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

  You should have received a copy of the GNU Lesser
  General Public License along with this program.  If not,
  see <http://www.gnu.org/licenses/>.  */

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REPOSITORY_HPP_
#define CONCURRO_REPOSITORY_HPP_

#include <set>
#include "types/typeinfo.h"
#include "Repository.h"
#include "RState.hpp"

namespace curr {

template<class Obj, class Par, class ObjId>
const AbstractRepositoryBase::Traits 
  RepositoryInterface<Obj, Par, ObjId>::traits
  ({::types::type<Obj>::name(), 
     ::types::type<Par>::name(), 
      ::types::type<ObjId>::name()});

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
  Obj* obj = 0;
  // <NB> cinfo.objectId is empty at the first call to
  // param
  ObjectCreationInfo cinfo;
  cinfo.repository = this;
  { 
    RLOCK (objectsM);

    const ObjId objId = allocate_new_object_id_internal
      (cinfo, param);

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

template <
  class Obj, 
  class Par, 
  template<class...> class ObjMap, 
  class ObjId
>
Obj* RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::replace_object 
  (const ObjId& id, const Par& param, bool freeMemory)
{
  using namespace ::types;

  RLOCK(objectsM);

  Obj* obj = 0;
  try {
    obj = objects->at (id);
  }
  catch (const std::out_of_range&) {
    throw exception<NoSuchId>(
      "No object with id [", id, "] exists in ",
      limit_tail<40>(type<RepositoryBase>::name())
    );
  }

  if (!obj)
    throw ::types::exception<RepositoryException>
     ("Program error");

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
::delete_object_by_id (const ObjId& id, bool freeMemory)
{
  using namespace ::types;

  Obj* ptr = 0;
  {
    RLOCK(objectsM);

    try {
      ptr = objects->at (id);
      delete_object_id(id);
      if (ptr == 0) 
        throw exception<NoSuchId>(
          "No object with id [", id, "] exists in ",
          limit<40>(type<RepositoryBase>::name())
        );
    }
    catch (const std::out_of_range&) {
      throw exception<NoSuchId>(
        "No object with id [", id, "] exists in ",
        limit<40>(type<RepositoryBase>::name())
      );
    }
  }
  //ptr->freeze();
  if (freeMemory) delete ptr;
}

template <class Obj, class Par, template<class...> class ObjMap, class ObjId>
Obj* RepositoryBase <Obj, Par, ObjMap, ObjId>
//
::get_object_by_id (const ObjId& id) const
{
  using namespace ::types;

  try { 
    RLOCK(objectsM);
    return objects->at (id);
  }
  catch (const std::out_of_range&)
  {
    throw exception<NoSuchId>(
      "No object with id [", id, "] exists in ",
      limit<40>(type<RepositoryBase>::name())
    );
  }
}

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
template<class Out, class Pred>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_pred (Out res, Pred p) const
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
    return RAxis<typename State::axis>::state_is (obj, state);
  }
protected:
  State state;
};

template<class Obj, class Par, template<class...> class ObjMap, class ObjId>
template<class Out, class State>
Out RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::get_object_ids_by_state 
  (Out res, const State& state) const
{
  return get_object_ids_by_pred<Out, StateMatch<Obj,
  State>> 
    (res, StateMatch<Obj, State> (state));
}

template <
  class Obj, class Par, template<class...> class ObjMap, 
  class ObjId
>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::for_each(std::function<void(Obj&)> f)
{
  RLOCK(objectsM);

  Obj* ptr;
  for (const auto& v : *objects)
    if ((ptr = Value(v)))
      f (*ptr);
}

template <
  class Obj, class Par, template<class...> class ObjMap, 
  class ObjId
>
void RepositoryBase<Obj, Par, ObjMap, ObjId>
//
::for_each(std::function<void(const Obj&)> f) const
{
  RLOCK(objectsM);

  Obj* ptr;
  for (const auto& v : *objects)
    if ((ptr = Value(v)))
      f (const_cast<const Obj&>(*ptr));
}

// Repository

template<class Obj, class Par, class ObjId>
ObjId Repository<Obj, Par, std::unordered_map, ObjId>
//
::allocate_new_object_id_internal(
  ObjectCreationInfo& oi, 
  const Par& param
)
{
  RLOCK(this->objectsM);

  ObjId id = param.get_id(oi);

  if (this->objects->find(id) != this->objects->end())
    throw ::types::exception<typename Parent::IdIsAlreadyUsed>
      ("The object id [", id, "] is used already.");

  return id;
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
  throw ::types::exception<SeveralObjects>(
    "The param leads to several objects creation, "
    "you should use create_several_objects."
  );
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
        const ObjId objId = 
          this->allocate_new_object_id_internal(cinfo, param);
        toString(objId, cinfo.objectId);

        // dynamic cast for use with inherited parameters
        obj = dynamic_cast<Obj*>
          (param.create_next_derivation (cinfo));
        SCHECK (obj);
        this->insert_object (objId, obj);
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

}

#endif
