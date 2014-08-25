/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-
***********************************************************

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
 * Log system switch.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_LOGGER_H
#define CONCURRO_LOGGER_H

#include <memory>
#include <string>
#include <limits>

namespace curr {

namespace logging {

/**
 * @addtogroup logging
 *
 * Just logging.
 * @{
 */

class Level;
using LevelPtr = Level;

/**
 * Defines the minimum set of levels recognized by the
 * system, that is <code>OFF</code>, <code>FATAL</code>,
 * <code>ERROR</code>, <code>WARN</code>,
 * <code>INFO</code>, <code>DEBUG</code> and
 * <code>ALL</code>.  <p>The <code>Level</code> class may
 * be subclassed to define a larger level set.
*/
class Level 
{
public:
  enum : int {
    OFF_INT = std::numeric_limits<int>::max(),
    FATAL_INT = 50000,
    ERROR_INT = 40000,
    WARN_INT = 30000,
    INFO_INT = 20000,
    DEBUG_INT = 10000,
    TRACE_INT = 5000,
    ALL_INT = std::numeric_limits<int>::min()
  };

  Level(int level_) : level(level_) {}

  static LevelPtr getAll();
  static LevelPtr getFatal();
  static LevelPtr getError();
  static LevelPtr getWarn();
  static LevelPtr getInfo();
  static LevelPtr getDebug();
  static LevelPtr getTrace();
  static LevelPtr getOff();

  int toInt() const { return level; }

protected:
  const int level;
};

class LocationInfo {};

class Logger;
using LoggerPtr = std::shared_ptr<Logger>;

#define LEVEL_DEBUG (::curr::logging::Level::getDebug())

/**
 * The logger concept.
 */
class Logger
{
public:
  bool isTraceEnabled() const;
  bool isDebugEnabled() const;
  bool isInfoEnabled() const;
  bool isWarnEnabled() const;
  bool isErrorEnabled() const;
  bool isFatalEnabled() const;

  /**
  Check whether this logger is enabled for a given
  Level passed as parameter.

  See also #isDebugEnabled.

  @return bool True if this logger is enabled for 
          <code>level</code>.
  */
  bool isEnabledFor(const LevelPtr& level) const;

  /**
  This method creates a new logging event and logs the
  event without further checks.
  @param level the level to log.
  @param message message.
  @param location location of source of logging request.
  */
  void forcedLog(
    const LevelPtr& level, 
    const std::string& message,
    const LocationInfo& location
  ) const;

  /**
  This method creates a new logging event and logs the
  event without further checks.
  @param level the level to log.
  @param message message.
  */
  void forcedLog(
    const LevelPtr& level, 
    const std::string& message
  ) const;

  /**
   * Get the logger name.
   * @return logger name as std::string.  
   */
  std::string getName() const;

  /**
   * Retrieve a logger by name in current encoding.
   * @param name logger name. 
   */
  static LoggerPtr getLogger(const std::string& name);

  LevelPtr getEffectiveLevel() const;
};

//! @}

} // logging

} // curr

#endif
