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

#ifndef CONCURRO_SSINGLETON_H_
#define CONCURRO_SSINGLETON_H_

#include "SException.h"

namespace curr {

//! @defgroup Singletons
//! @{

/** 
 * @class NotExistingSingleton
 * An exception raised when somebody
 * tries get an SSingleton instance when no one copy of
 * SSingleton<T> exists.
 */ 
class NotExistingSingleton;

/** 
 * @class MustBeSingleton
 * An exception raised when somebody tries create more
 * than once instance of SSingleton.
 */ 
class MustBeSingleton;

/**
 * Base class for classes that can have only one instance
 * parametrised by the actual singleton class, use as:
 * class MyClass : public SSingleton<MyClass>
 *
 * @dot
 * digraph {
 *    start [shape = point]; 
 *    stop [shape = point];
 *    destroyed;
 * }
 * @enddot
 *
 */
template<class T>
class SSingleton
{
public:
  //! One and only one class instance must be created with
  //! this function.
  //! @exception MustBeSingleton a copy of SSingleton<T>
  //! already exists.
  SSingleton();

  virtual ~SSingleton();

  //! Return the reference to the class instance. 
  //! @exception NotExistingSingleton If no
  //! class is crated with SSingleton() raise exception.
  static T & instance();

  // Added by slod to prevent assertion with MainWin in
  // Common::ErrorMessage.
  // It is not true singleton (why?) and thus we need a
  // trick.
  static bool isConstructed ()
  {
     return _instance != NULL;
  }

private:
  static T * _instance;
};

/**
 * Extends SSingleton to allow auto-construct it by the
 * RAutoSingleton::instance() call.
 */
template<class T>
class SAutoSingleton : public SSingleton<T>
{
public:
  static T & instance ();
};

//! @}

}
#endif
