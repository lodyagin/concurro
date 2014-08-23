/* -*-coding: mule-utf-8-unix; fill-column: 58; -*- *******

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

#ifndef CONCURRO_OBJECTWITHLOGGING_H_
#define CONCURRO_OBJECTWITHLOGGING_H_

#include <map>
#include "types/typeinfo.h"
#include "types/enum_map.h"
#include "Logger.h"

namespace curr {

//! @addtogroup logging
//! @{

class LogParams;

namespace object_with_logging_ {

template<class... Vs>
struct disabler;

template<>
struct disabler<>
{
  disabler(LogParams& pars) {}
  void disable() {}
};

template<class V, class... Vs>
struct disabler<V, Vs...> : disabler<Vs...>
{
  disabler(LogParams& pars_) 
    : disabler<Vs...>(pars_),
      pars(pars_) {}

  void disable()
  {
    pars[V()] = true;
    disabler<Vs...>(pars).disable();
  }

  LogParams& pars;
};

} // object_with_logging_

class ObjectWithLogging;

//! They refine logging
struct LogParams
{
  //! disabled params marked true
  ::types::enum_map<bool, std::map> pars;
  ObjectWithLogging* log_obj;

  LogParams(ObjectWithLogging* obj)
    : log_obj(obj)
  {
    assert(log_obj);
  }

  template<class... EnumVal>
  void disable()
  {
    using namespace object_with_logging_;
    disabler<EnumVal...>(*this).disable();
  }

  //! Clear all disable bits
  void enable_all()
  {
    pars.clear();
  }

  template<class EnumVal>
  bool& operator[](EnumVal val)
  {
    return pars.operator[](val);
  }

  //! Merge disabled 
  LogParams& operator|=(const LogParams& o)
  {
    pars |= o.pars;
    return *this;
  }
};

/**
 * An abstract parent of "Object with logging" - an entity
 * with its own logging namespace.
 */
class ObjectWithLogging
{
public:
  //! It should be overriden. The default implementation
  //! is for logging from constructors to prevent a pure
  //! virtual function call
  virtual logging::LoggerPtr logger() const;

  //! The default implementation
  virtual std::string object_name() const
  {
    return ::types::demangled_name(typeid(*this));
  }
};

class ObjectWithLogParams : public virtual ObjectWithLogging
{
  mutable LogParams log_params_;

public:
  ObjectWithLogParams() : log_params_(this) {}

  virtual LogParams& log_params() const
  {
    return log_params_;
  }
};

//! @}

}

#endif

