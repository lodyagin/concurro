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

#include "Logger.h"

namespace curr {

namespace logging {

LevelPtr Level::getAll()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(ALL_INT);
  return the_level;
}

LevelPtr Level::getFatal()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(FATAL_INT);
  return the_level;
}

LevelPtr Level::getError()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(ERROR_INT);
  return the_level;
}

LevelPtr Level::getWarn()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(WARN_INT);
  return the_level;
}

LevelPtr Level::getInfo()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(INFO_INT);
  return the_level;
}

LevelPtr Level::getDebug()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(DEBUG_INT);
  return the_level;
}

LevelPtr Level::getTrace()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(TRACE_INT);
  return the_level;
}

LevelPtr Level::getOff()
{
  static LevelPtr the_level = 
    std::make_shared<Level>(OFF_INT);
  return the_level;
}


bool Logger::isTraceEnabled() const
{
  return getEffectiveLevel()->toInt() <= Level::TRACE_INT;
}

bool Logger::isDebugEnabled() const
{
  return getEffectiveLevel()->toInt() <= Level::DEBUG_INT;
}

bool Logger::isInfoEnabled() const
{
  return getEffectiveLevel()->toInt() <= Level::INFO_INT;
}

bool Logger::isWarnEnabled() const
{
  return getEffectiveLevel()->toInt() <= Level::WARN_INT;
}

bool Logger::isErrorEnabled() const
{
  return getEffectiveLevel()->toInt() <= Level::ERROR_INT;
}

bool Logger::isFatalEnabled() const
{
  return getEffectiveLevel()->toInt() <= Level::FATAL_INT;
}

LevelPtr Logger::getEffectiveLevel() const
{
  return Level::getDebug();
}

std::string Logger::getName() const
{
  return "Logger.cpp";
}

LoggerPtr Logger::getLogger(const std::string& name)
{
  static LoggerPtr the_logger = std::make_shared<Logger>();
  return the_logger;
}

} // logging

} // curr

