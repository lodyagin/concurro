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

#ifndef CONCURRO_GUARD_H_
#define CONCURRO_GUARD_H_

#include "REvent.h"
#include "RState.h"

namespace curr {

//! @addtogroup repositories
//! @{

DECLARE_AXIS(NReaders1WriterAxis, StateAxis);

#if 0 // NReaders1WriterAxis or other template parameter
      // axis must be implemented in the guarded object

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
  DECLARE_EVENT(NReaders1WriterAxis, discharged);
  DECLARE_EVENT(NReaders1WriterAxis, charged);
  DECLARE_EVENT(NReaders1WriterAxis, readers_entered);

public:
  //! @cond
  DECLARE_STATES(NReaders1WriterAxis, State);
  DECLARE_STATE_CONST(State, discharged);
  DECLARE_STATE_CONST(State, charged);
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

  NReaders1WriterGuard() : NReaders1WriterGuard(nullptr) {}

  NReaders1WriterGuard(nullptr_t object) 
   : RObjectWithEvents<NReaders1WriterAxis>(S(discharged)),
      CONSTRUCT_EVENT(discharged),
      CONSTRUCT_EVENT(charged),
      CONSTRUCT_EVENT(readers_entered),
      readers_cnt(0),
      obj(nullptr)
  {
  }

  NReaders1WriterGuard(T* object) 
    : RObjectWithEvents<NReaders1WriterAxis>(S(charged)),
      CONSTRUCT_EVENT(discharged),
      CONSTRUCT_EVENT(charged),
      CONSTRUCT_EVENT(readers_entered),
      readers_cnt(0),
      obj(object)
  {
    SCHECK(obj);
  }

  //NReaders1WriterGuard(NReaders1WriterGuard&& g) noexcept
  //  : obj(g.

  NReaders1WriterGuard& operator=
    (const NReaders1WriterGuard&) = delete;

  //! You should call discharge() before the destructor
  //! because discharge can throw the timeout exception.
  ~NReaders1WriterGuard()
  {
    discharge();
  }

  void swap(NReaders1WriterGuard& g) noexcept
  {
    std::swap(obj, g.obj);
  }

  //! Charge with an object
  void charge(T* object)
  {
    SCHECK(object);
    obj = object;
    move_to(*this, S(charged));
  }

  void discharge()
  {
    static const std::set<RState<NReaders1WriterAxis>> 
      from_set { S(discharged), S(charged) };

    wait_and_move(
      *this, 
      from_set,
      E(ready_to_discharge),
      S(discharged),
      wait_m
    );
    obj = nullptr;
  }

  CompoundEvent is_terminal_state() const override
  {
    return CompoundEvent{ E(discharged) };
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
    { E(charged), E(readers_entered) };

  CompoundEvent E(ready_to_discharge) = 
    { E(discharged), E(charged) };

  void start_read_op()
  {
    static const std::set<RState<NReaders1WriterAxis>> 
      from_set { S(charged), S(readers_entered) };

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
      move_to(*this, S(charged));
  }

  void start_write_op()
  {
    wait_and_move(*this, E(charged), S(writer_entered), 
                  wait_m);
    assert(0 == readers_cnt);
  }

  void stop_write_op()
  {
    move_to(*this, S(charged));
  }

  size_t get_readers_cnt() const
  {
    return readers_cnt;
  }

  //reduce memory footprint
  //CompoundEvent E(terminal_state) = { E(charged) };

  std::atomic<size_t> readers_cnt;
  T* obj;
};
#endif

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

#if 0 // FIXME review with newest repository design

  NoGuard(const NoGuard& g) = default;
  NoGuard& operator=(const NoGuard& g) = default;

#endif 

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

  const T* get() const
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

//! @}

}

#endif


