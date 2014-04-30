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

#ifndef CONCURRO_RHOLDER_H_
#define CONCURRO_RHOLDER_H_

#include "RObjectWithStates.h"
#include "RState.h"
#include "REvent.h"
#include <atomic>

namespace curr {

//! @addtogroup repositories
//! @{

DECLARE_AXIS(NReaders1WriterAxis, StateAxis);

//FIXME may be inconsistent with RHolder copy/move
/**
 * An object guard which do not allow several "write"
 * operations being performed simultaneously on a guarded
 * object of type T (a several readers - single writer
 * conception).
 * \tparam wait_m Wait time in milliseconds (-1 means
 * infinite) 
 */
template<class T, int wait_m = 1000>
class NReaders1WriterGuard
  : public RObjectWithEvents<NReaders1WriterAxis>
{
  DECLARE_EVENT(NReaders1WriterAxis, free);
  DECLARE_EVENT(NReaders1WriterAxis, readers_entered);

public:
  //! @cond
  DECLARE_STATES(NReaders1WriterAxis, State);
  DECLARE_STATE_CONST(State, free);
  DECLARE_STATE_CONST(State, reader_entering);
  DECLARE_STATE_CONST(State, readers_entered);
  DECLARE_STATE_CONST(State, reader_exiting);
  DECLARE_STATE_CONST(State, writer_entered);
  //! @endcond

  class ReadPtr
  {
  public:
    ReadPtr(const NReaders1WriterGuard* g) : guard(g)
    {
      const_cast<NReaders1WriterGuard*>(guard)
        -> start_read_op();
    }

    ~ReadPtr()
    {
      const_cast<NReaders1WriterGuard*>(guard)
        -> stop_read_op();
    }

    const T* operator->() const
    {
      return guard->obj;
    }

  private:
    const NReaders1WriterGuard* guard;
  };

  class WritePtr
  {
  public:
    WritePtr(NReaders1WriterGuard* g) : guard(g)
    {
      guard->start_write_op();
    }

    ~WritePtr()
    {
      guard->stop_write_op();
    }

    T* operator->()
    {
      return guard->obj;
    }

  private:
    NReaders1WriterGuard* guard;
  };

  NReaders1WriterGuard(T* object) 
    : RObjectWithEvents<NReaders1WriterAxis>(S(free)),
      CONSTRUCT_EVENT(free),
      CONSTRUCT_EVENT(readers_entered),
      readers_cnt(0),
      obj(object)
  {
    assert(obj);
  }

  CompoundEvent is_terminal_state() const override
  {
    return E(terminal_state);
  }

  //! Read access to the guarded object
  const ReadPtr operator->() const
  {
    return ReadPtr(this);
  }

  //! Write access to the guarded object
  WritePtr operator->()
  {
    return WritePtr(this);
  }

protected:

  CompoundEvent E(ready_to_new_reader) = 
    { E(free), E(readers_entered) };

  void start_read_op()
  {
    static const std::set<RState<NReaders1WriterAxis>> 
      from_set { S(free), S(readers_entered) };

    wait_and_move
      (*this, from_set, 
       E(ready_to_new_reader), S(reader_entering),
       wait_m);

    ++readers_cnt;

    move_to(*this, S(readers_entered));
  }

  void stop_read_op()
  {
    move_to(*this, S(reader_exiting));
    if (--readers_cnt)
      move_to(*this, S(readers_entered));
    else
      move_to(*this, S(free));
  }

  void start_write_op()
  {
    wait_and_move(*this, E(free), S(writer_entered), 
                  wait_m);
    assert(0 == readers_cnt);
  }

  void stop_write_op()
  {
    move_to(*this, S(free));
  }

  size_t get_readers_cnt() const
  {
    return readers_cnt;
  }

  CompoundEvent E(terminal_state) = { E(free) };

  std::atomic<size_t> readers_cnt;
  T* obj;
};

//! The dummy guard - no guard at all, just guard interface
template<class T, int>
class NoGuard
{
public:
  using WritePtr = T*;
  using ReadPtr = const T*;

  NoGuard() : NoGuard(nullptr) {}

  NoGuard(nullptr_t object) : obj(nullptr) {}

  NoGuard(T* object) : obj(object) 
  {
    SCHECK(obj);
  }

  NoGuard(NoGuard&& g) noexcept : obj(g.discharge()) {}

  NoGuard& operator=(NoGuard&& g) noexcept
  {
    obj = g.discharge();
  }

  ~NoGuard() { discharge(); }

  NoGuard& charge(T* object)
  {
    SCHECK(object);
    obj = object;
    return *this;
  }

  void swap(NoGuard& g) noexcept
  {
    std::swap(obj, g.obj);
  }

  T* discharge() noexcept
  {
    T* res = obj; // NB res can be nullptr
    obj = nullptr;
    return res;
  }

  T* get()
  {
    return obj;
  }

  operator bool() const
  {
    return obj;
  }

  T* operator->()
  {
    return obj;
  }
   
  const T* operator->() const
  {
    return obj;
  }

protected:
  T* obj;
};

enum class HolderType { 
  Singular, //<! a view of one object
  Plural    //<! a view of an array of objects
};

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
  class Guard = NoGuard,

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


