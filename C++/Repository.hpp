/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_REPOSITORY_HPP_
#define CONCURRO_REPOSITORY_HPP_

#include "Repository.h"
#include "RState.hpp"
#include <set>

namespace curr {

#define CURRINT_REPOSITORY_TEMPL_ template \
< \
  class Obj, \
  class Par, \
  template<class...> class ObjMap, \
  class ObjId, \
  template<class, int> class Guard, \
  int wait_m \
>
#define CURRINT_REPOSITORY_T_ Obj,Par,ObjMap,ObjId,Guard,wait_m

template<
  class Obj, 
  class Par, 
  class ObjId, 
  template<class,int> class Guard, 
  int wait_m
>
const AbstractRepositoryBase::Traits 
RepositoryInterface<Obj, Par, ObjId, Guard, wait_m>
::traits(
  { curr::type<Obj>::name(), 
    curr::type<Par>::name(), 
    curr::type<ObjId>::name() 
  }
);

CURRINT_REPOSITORY_TEMPL_
RepositoryBase<CURRINT_REPOSITORY_T_>
//
::~RepositoryBase ()
{
  std::for_each(
    objects->begin (), 
    objects->end (),
    Destructor<
      Obj, 
      Par, 
      ObjMap, 
      ObjId, 
      Guard,
      wait_m,
      typename RepositoryMapType
        <Obj, ObjId, ObjMap, Guard, wait_m>
          ::Map::value_type
    > (this)
  );
  delete objects;
}

CURRINT_REPOSITORY_TEMPL_
Guard<Obj,wait_m>& RepositoryBase<CURRINT_REPOSITORY_T_>
//
::create_object (const Par& param)
{
  Guard<Obj,wait_m>* res = nullptr;
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
    res = &insert_object (objId, obj);
  }
  LOG_TRACE(log, "Object " << *obj << " is created.");

  if (cinfo.objectCreated)
    cinfo.objectCreated->set(); 
    // <NB> after inserting into repo
    // and unlocking

  assert(res);
  return *res;
}

CURRINT_REPOSITORY_TEMPL_
Guard<Obj,wait_m>& RepositoryBase<CURRINT_REPOSITORY_T_>
//
::replace_object 
  (const ObjId& id, const Par& param/*, bool freeMemory*/)
{
  RLOCK(objectsM);

  Obj* obj = 0;
  try {
    obj = objects->at (id).get();
  }
  catch (const std::out_of_range&) {
    THROW_EXCEPTION(NoSuchId, id);
  }

  if (!obj) THROW_PROGRAM_ERROR;
  //obj->freeze();

  (*objects)[id].charge(
    dynamic_cast<Obj*>(param.transform_object (obj))
  );

  SCHECK ((*objects)[id]);

  if ((*objects)[id].get() != obj)
    if (true /*freeMemory*/)
      delete obj;

  return (*objects)[id];
}

CURRINT_REPOSITORY_TEMPL_
void RepositoryBase<CURRINT_REPOSITORY_T_>
//
::delete_object (Guard<Obj,wait_m>& obj/*, bool freeMemory*/)
{
  assert (obj);
  const ObjId objId = fromString<ObjId> 
    (obj->universal_id());
//    (obj.operator->()->universal_id());

  delete_object_by_id (objId/*, freeMemory*/);
}

CURRINT_REPOSITORY_TEMPL_
void RepositoryBase<CURRINT_REPOSITORY_T_>
//
::delete_object_by_id (const ObjId& id/*, bool freeMemory*/)
{
  Obj* ptr = 0;
  {
    RLOCK(objectsM);

    try {
      ptr = objects->at (id).get();
      delete_object_id(id);
      if (ptr == 0) 
        THROW_EXCEPTION(NoSuchId, id);
    }
    catch (const std::out_of_range&) {
      THROW_EXCEPTION(NoSuchId, id);
    }
  }
  //ptr->freeze();
  if (true /*freeMemory*/) delete ptr;
}

CURRINT_REPOSITORY_TEMPL_
Guard<Obj,wait_m>& RepositoryBase <CURRINT_REPOSITORY_T_>
//
::get_object_by_id (const ObjId& id) const
{
  try { 
    RLOCK(objectsM);
    return objects->at (id);
  }
  catch (const std::out_of_range&)
  {
    THROW_EXCEPTION_PLACE(get_object_by_id, NoSuchId, id);
  }
}

CURRINT_REPOSITORY_TEMPL_
template<class Out, class Pred>
Out RepositoryBase<CURRINT_REPOSITORY_T_>
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

CURRINT_REPOSITORY_TEMPL_
template<class Out, class State>
Out RepositoryBase<CURRINT_REPOSITORY_T_>
//
::get_object_ids_by_state 
  (Out res, const State& state) const
{
  return get_object_ids_by_pred<Out, StateMatch<Obj,
  State>> 
    (res, StateMatch<Obj, State> (state));
}

CURRINT_REPOSITORY_TEMPL_
void RepositoryBase<CURRINT_REPOSITORY_T_>
//
::for_each(std::function<void(Guard<Obj,wait_m>&)> f)
{
  RLOCK(objectsM);

  for (auto& v : *objects)
  {
    Value vv(v);
    Guard<Obj,wait_m>& g = vv;
    if (g)
      f(g);
  }
}

CURRINT_REPOSITORY_TEMPL_
void RepositoryBase<CURRINT_REPOSITORY_T_>
//
::for_each(std::function<void(const Guard<Obj,wait_m>&)> f) const
{
  RLOCK(objectsM);

  for (const auto& v : *objects)
  {
    const Guard<Obj,wait_m>& g = ConstValue(v);
    if (g)
      f(g);
      //f (const_cast<const Guard<Obj,wait_m>&>(*ptr));
  }
}

/*=====================================*/
/*========== SparkRepository ==========*/
/*=====================================*/

#define CURRINT_SPARK_REPOSITORY_TEMPL_ template \
< \
  class Obj, \
  class Par, \
  template<class...> class ObjMap, \
  class ObjId, \
  template<class...> class List, \
  template<class, int> class Guard, \
  int wait_m \
>
#define CURRINT_SPARK_REPOSITORY_T_ \
  Obj,Par,ObjMap,ObjId,List,Guard,wait_m

CURRINT_SPARK_REPOSITORY_TEMPL_
Guard<Obj,wait_m>& SparkRepository<CURRINT_SPARK_REPOSITORY_T_>
//
::create_object (const Par& param)
{
  //THROW_EXCEPTION(SeveralObjects, void);
  throw SeveralObjects();
}

CURRINT_SPARK_REPOSITORY_TEMPL_
List<Guard<Obj,wait_m>*> SparkRepository<CURRINT_SPARK_REPOSITORY_T_>
//
::create_several_objects(Par& param)
{
  List<Guard<Obj,wait_m>*> out;
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
        out.push_back(&this->insert_object(objId, obj));
        LOG_TRACE(log, 
                  "Object " << *obj << " is created.");
        
        if (cinfo.objectCreated)
          cinfo.objectCreated->set();
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
