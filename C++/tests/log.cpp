// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#include "Logger.h"
#include "log.h"

namespace curr {
namespace logging {

void Logger::forcedLog(
  const LevelPtr& level, 
  const std::string& message
) const
{
  lg::stream::instance() << message << std::endl;
}

void Logger::forcedLog(
  const LevelPtr& level, 
  const std::string& message,
  const LocationInfo&
) const
{
  lg::stream::instance() << message << std::endl;
}

} // logging
} // curr