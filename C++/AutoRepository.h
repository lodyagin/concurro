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

#ifndef CONCURRO_AUTOREPOSITORY_H_
#define CONCURRO_AUTOREPOSITORY_H_

#include "Repository.h"
#include "SSingleton.h"
#include "SCommon.h"
#include <map>

namespace curr {

/**
 * An AutoSingleton which allows have one and only one
 * Repository for each Object/ObjectId par. <NB> There are
 * can be different repositories for the same Object type
 * differs by an identification schema (ObjectId).
 * @ingroup repositories
 */
template<class Object, class ObjectId>
class AutoRepository 
: 
  public Repository<
    Object, typename Object::Par, std::map, ObjectId>,

  public SAutoSingleton<AutoRepository<Object, ObjectId>>
{
public:
  using Parent = Repository<
    Object, typename Object::Par, std::map, ObjectId>;

  AutoRepository()
  : Parent(SFORMAT(
             "AutoRepository<" 
             << typeid(Object).name()
             << "[" << typeid(ObjectId).name() << "]>"),
           0 // not used for map-based Repository
           )
  {
  }
};

}

#endif

