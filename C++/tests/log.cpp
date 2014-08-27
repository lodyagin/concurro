// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#include "hardcore/stack.hpp"
#include "Logger.h"
#include "log.h"

//#define LOG_STACK

namespace curr {
namespace logging {

void Logger::forcedLog(
  const LevelPtr& level, 
  const std::string& message
) const
{
  lg::stream::instance() << ++lg::stream::event_id << '\t'
    << message 
#ifdef LOG_STACK
    << '\n' << hc::stack::returns()
#endif
    << std::endl;
}

void Logger::forcedLog(
  const LevelPtr& level, 
  const std::string& message,
  const LocationInfo&
) const
{
  lg::stream::instance() << ++lg::stream::event_id << '\t'
    << message 
#ifdef LOG_STACK
    << '\n' << hc::stack::returns()
#endif
    << std::endl;
}

} // logging
} // curr

namespace lg {

int64_t stream::event_id;

} // lg
