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

#ifndef CONCURRO_OBJECTWITHLOGGING_H_
#define CONCURRO_OBJECTWITHLOGGING_H_

#include "Logger.h"

namespace curr {

//! @addtogroup logging
//! @{

/**
 * An abstract parent of "Object with logging" - an entity
 * with its own logging namespace.
 */
class ObjectWithLogging
{
public:
  //! It should be overriden. The default implementation
  //! is for logging from constructors to prevent a pure
  //! virtual function call.
  virtual logging::LoggerPtr logger() const;
};

//! @}

}

#endif

