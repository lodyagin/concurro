/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

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

#ifndef CONCURRO_RHOLDER_H_
#define CONCURRO_RHOLDER_H_

#include "RObjectWithStates.h"
#include "RState.h"
#include "REvent.h"
#include "Guard.h"
#include <atomic>

namespace curr {

//! @addtogroup repositories
//! @{

DECLARE_AXIS(HolderAxis, StateAxis);

/**
 * It is a "view" of an object T in a repository.
 *
 * \tparam Obj is a (polymorphic) type of a repository
 * object, T is the same or a derivation of Obj
 */
template
<
  class T,
  class Obj = T,

  template<class, int>
  class Guard = NReaders1WriterGuard,

  int wait_m = 1000
>
class RHolder : public RObjectWithStates<HolderAxis>
{
public:
  //typedef typename Rep::Object Obj;
  //typedef typename Rep::ObjectId Id;
  typedef typename Obj::Par Par;

  //! Create a new object in the repository Rep and create
  //! the first holder to it.
  //! An AutoRepository is selected based on the (Obj,Id)
  //! pair
  template<class Id>
  RHolder(const Par& par);

  //! Charge with the object with the specified id.
  //! An AutoRepository is selected based on the (Obj,Id)
  //! pair
  //! @throw Rep::NoSuchId
  template<class Id>
  RHolder(const Id& id);

#if 0
  //! Double holder to the same object
  RHolder(const RHolder&);

  //! Do not double holder to the object
  RHolder(RHolder&&);

  //! Remove the holder. When the last object holder is
  //! removed object will be frozen
  ~RHolder();

  //! Double holder to the same object
  RHolder& operator=(const RHolder&);

  //! Do not double holder to the object
  RHolder& operator=(RHolder&&);
#endif

  //! Make a read-only object call (see
  //! NReaders1WriterGuard) 
  const typename Guard<T,wait_m>::ReadPtr operator->() const
  {
    return guarded.operator->();
  }

  //! Make a read/write object call (see
  //! NReaders1WriterGuard) 
  typename Guard<T, wait_m>::WritePtr operator->()
  {
    return guarded.operator->();
  }

  RHolder* operator&() = delete;

protected:
  // The guarded object
  Guard<T, wait_m> guarded;
};

#if 0
template
<
  class Obj, class Par, template<class...> class ObjMap, 
  class ObjId, class Holder, class Guard
>
class ProtectedRepositoryInterface
  : public AbstractRepositoryBase
{
public:
  virtual RHolder create_object
};

template< 
  class Obj, 
  class Par, 
  template<class...> class ObjMap, 
  class ObjId,
  class Holder,
  class Guard
>
class ProtectedRepository  
  : public Repository<Obj, Par, ObjMap, ObjId>
{
  ProtectedRepository
    (const std::string& repository_name,
     size_t initial_capacity);
};
#endif

//! @}

}

#endif


