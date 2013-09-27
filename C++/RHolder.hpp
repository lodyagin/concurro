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

#ifndef CONCURRO_RHOLDER_HPP_
#define CONCURRO_RHOLDER_HPP_

#include "RHolder.h"
#include "RState.hpp"

namespace curr {

#define NReaders1WriterGuardTW_  NReaders1WriterGuard<T,w>

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, free);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, reader_entering);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, readers_entered);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, reader_exiting);

template<class T, int w>
DEFINE_STATE_CONST(NReaders1WriterGuardTW_, State, writer_entered);

#if 0
template<class T>
RHolder<T>::RHolder(const RHolder<T>& h)
{
  obj = h.obj;
}

RHolder(RHolder&&);

~RHolder();

RHolder& operator=(const RHolder&);
RHolder& operator=(RHolder&&);
#endif

}

#endif


