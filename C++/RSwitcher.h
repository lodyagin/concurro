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

#ifndef RSWITCHER_H_
#define RSWITCHER_H_

#include <cstdatomic>

namespace curr {

/**
 * @addtogroup synchronization
 * @{
 */

/**
 * A value with wait-free update implementation.
 */
template<class T, class TUpdate>
class RSwitcher
{
public:
  /// Update the element.
  void update(const TUpdate&);
  /// Get the last updated state.
  T get() const;
};

//! @}

}
#endif
