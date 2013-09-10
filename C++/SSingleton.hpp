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

#ifndef CONCURRO_SSINGLETON_HPP_
#define CONCURRO_SSINGLETON_HPP_

#include "SSingleton.h"
#include "Logging.h"
#include "SCheck.h"

namespace curr {

DEFINE_EXCEPTION(
  NotExistingSingleton, 
  "Somebody tries to get an SSingleton::instance() when "
  "no one available");

DEFINE_EXCEPTION(
  MustBeSingleton, 
  "Somebody tries to create more than one "
  "SSingleton instance");

template<class T>
SSingleton<T>::SSingleton()
{
  if (_instance)
    THROW_EXCEPTION(MustBeSingleton);

  _instance = static_cast<T *>(this);
}

template<class T>
SSingleton<T>::~SSingleton()
{
  _instance = nullptr;
}

template<class T>
inline T & SSingleton<T>::instance()
{
  if (!_instance)
    THROW_EXCEPTION(NotExistingSingleton);
    
  return *_instance;
}

template<class T>
T * SSingleton<T>::_instance(nullptr);

template<class T>
T& SAutoSingleton<T>::instance ()
{
  if (!SSingleton<T>::isConstructed ()) 
    new T (); 

  return SSingleton<T>::instance ();
}

}

#endif

