/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
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
    : RObjectWithEvents<NReaders1WriterAxis>("free"),
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

#if 0
enum class HolderType { 
  Singular, //<! a view of one object
  Plural    //<! a view of an array of objects
};

DECLARE_AXIS(HolderAxis, StateAxis);

/**
 * It is a "view" of an object T in a repository.
 */
template<
  class T, 
  template <class T> class Guard = NReaders1WriterGuard
>
class RHolder : public RObjectWithStates<HolderAxis>
{
public:
  RHolder(const RHolder&);
  RHolder(RHolder&&);
  ~RHolder();

  RHolder& operator=(const RHolder&);
  RHolder& operator=(RHolder&&);

protected:
  Guard<T> guard;
};
#endif

//! @}

}

#endif


