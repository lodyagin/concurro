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

#ifndef CONCURRO_AUTOREPOSITORY_H_
#define CONCURRO_AUTOREPOSITORY_H_

#include "SSingleton.h"
#include "Repository.h"
#include "SCommon.h"
#include <map>

namespace curr {

/**
 * An AutoSingleton which allows have one and only one
 * Repository for each Object/ObjectId par, so there are
 * can be different repositories for the same Object type
 * differs by an identification schema (ObjectId).
 *
 * It is final for prevent SAutoSingleton::instance return
 * incomplete class.
 *
 * @ingroup repositories
 */
template<class Object, class ObjectId>
class AutoRepository final
  : public SAutoSingleton
      <AutoRepository<Object, ObjectId>>,
    public virtual RepositoryInterface
      <Object, typename Object::Par, ObjectId>
{
public:
  typedef typename Object::Par Par;
  typedef RepositoryInterface<Object, Par, ObjectId> RepI;
  typedef SAutoSingleton<AutoRepository<Object, ObjectId>>
    AutoSingleton;

  template<template <class...> class Cont>
  using Rep = Repository<Object, Par, Cont, ObjectId>;

  AutoRepository()
    : rep(new Rep<std::map>(curr::sformat(
          "AutoRepository<",
          curr::type<Object>::name(),
          "[", curr::type<ObjectId>::name(), "]>"),
          0))
  {
    assert(rep);
    this->complete_construction();
  }

  size_t size() const override
  {
    return rep->size();
  }

  Object* create_object (const Par& param) override
  {
    return rep->create_object(param);
  }
  
  void delete_object(Object* obj, bool freeMemory) override
  {
    rep->delete_object(obj, freeMemory);
  }

  void delete_object_by_id
    (const ObjectId& id, bool freeMemory) override
  {
    rep->delete_object_by_id(id, freeMemory);
  }

  //! \exception NoSuchId
  Object* get_object_by_id (const ObjectId& id) const 
    override
  {
    return rep->get_object_by_id(id);
  }

  Object* replace_object 
    (const ObjectId& id, 
     const Par& param, 
     bool freeMemory) 
    override
  {
    return rep->replace_object(id, param, freeMemory);
  }
  
  ObjectId allocate_new_object_id (const Par& par)
  {
    return rep->allocate_new_object_id(par);
  }

  void for_each
    (std::function<void(Object&)> f) override
  {
    return rep->for_each(f);
  }

  void for_each
    (std::function<void(const Object&)> f) const override
  {
    return const_cast<const RepI*>(rep)->for_each(f);
  }

  void clear()
  {
    for_each(Destroy(*this));
  }

  // call the method for all objects in the repo
  // TODO move to Repository
  template<class Method, class... Args>
  void global_call_method(Method&& m, Args&&... args)
  {
    for_each(std::bind(std::forward<Method>(m), _1, std::forward<Args>(args)...));
  }

  //! Init with the specified Cont type.
  template <
    template<class...> class Cont, 
    size_t initial_capacity = 100
  >
  static void init()
  {
    new AutoRepository(new Rep<Cont>(sformat(
          "AutoRepository<",
          curr::type<Object>::name(),
          "[", curr::type<ObjectId>::name(), "]"),
        initial_capacity));
  }

protected:
  RepI* rep;

  AutoRepository(RepI* r) : rep(r)
  {
    assert(rep);
  }
};

}

#endif


