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

#ifndef CONCURRO_RHOLDER_HPP_
#define CONCURRO_RHOLDER_HPP_

#include "RHolder.h"
#include "RState.hpp"
#include "AutoRepository.h"

namespace curr {

#if 0
#define NReaders1WriterGuardTW_  NReaders1WriterGuard<T,w>

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, free);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, 
                   reader_entering);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, 
                   readers_entered);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, 
                   reader_exiting);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, 
                   writer_entered);
#endif

// RHolder impl

#define CURRINT_HOLDER_TEMPL_ template \
< \
  class T, \
  class Obj, \
  template <class, int> class Guard, \
  int wait_m \
>
#define CURRINT_HOLDER_T_ T,Obj,Guard,wait_m


CURRINT_HOLDER_TEMPL_
template<class Id>
RHolder<CURRINT_HOLDER_T_>
//
::RHolder(const Par& par)
  : //RObjectWithStates<HolderAxis>("charged"),
    guarded(AutoRepository<Obj,Id>::instance()
      . template create<T>(par))
{
}

CURRINT_HOLDER_TEMPL_
template<class Id>
RHolder<CURRINT_HOLDER_T_>
//
::RHolder(const Id& id)
  : //RObjectWithStates<HolderAxis>("charged"),
    guarded(AutoRepository<Obj,Id>::instance()
      . template get_by_id<T>(id))
{
}

}

#endif


