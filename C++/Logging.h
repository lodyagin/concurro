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
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_LOGGING_H_
#define CONCURRO_LOGGING_H_
//#pragma warning(disable: 4250 4251)

#include <sstream>
#include <string>
#include <typeinfo>
#include <algorithm>
#include <atomic>
#include <utility>
#include "SSingleton.h"
#include "SCommon.h"
#include "Logging.h"
#ifdef USE_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/spi/location/locationinfo.h>
// try to fix __cxa_initialize order
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/spi/location/locationinfo.h>
#else
#define LOG4CXX_UNLIKELY(b) __builtin_expect((b), 0)
#define LOG4CXX_LOCATION logging::LocationInfo()
#endif
//
#include "ObjectWithLogging.h"

// Define a custom log macros for put streams into log
#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOGGER_DEBUG_LOC(log, stream_expr, loc) do {                    \
    if (LOG4CXX_UNLIKELY((log) && (log)->isDebugEnabled())) {                    \
    std::ostringstream oss_;                  \
    { oss_ << stream_expr ; }                                  \
    (log)->forcedLog(logging::Level::getDebug(), oss_.str(), loc); }} while (0)
#else
#define LOGGER_DEBUG_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 5000 
#define LOGGER_TRACE_LOC(log, stream_expr, loc) do {                    \
   if (LOG4CXX_UNLIKELY((log) && (log)->isTraceEnabled())) {            \
    std::ostringstream oss_;                  \
    { oss_ << stream_expr ; }                                  \
    (log)->forcedLog(logging::Level::getTrace(), oss_.str(), loc); }} while (0)
#else
#define LOGGER_TRACE_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 20000 
#define LOGGER_INFO_LOC(log, stream_expr, loc) do {                      \
   if (LOG4CXX_UNLIKELY((log) && (log)->isInfoEnabled())) {            \
    std::ostringstream oss_;                  \
    { oss_ << stream_expr ; }                                  \
    (log)->forcedLog(logging::Level::getInfo(), oss_.str(), loc); }} while (0)
#else
#define LOGGER_INFO_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 30000 
#define LOGGER_WARN_LOC(log, stream_expr, loc) do {                      \
   if (LOG4CXX_UNLIKELY((log) && (log)->isWarnEnabled())) {            \
    std::ostringstream oss_;                  \
    { oss_ << stream_expr ; }                                  \
    (log)->forcedLog(logging::Level::getWarn(), oss_.str(), loc); }} while (0)
#else
#define LOGGER_WARN_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 40000 
#define LOGGER_ERROR_LOC(log, stream_expr, loc) do {                    \
   if (LOG4CXX_UNLIKELY((log) && (log)->isErrorEnabled())) {            \
    std::ostringstream oss_;                  \
    { oss_ << stream_expr ; }                                  \
    (log)->forcedLog(logging::Level::getError(), oss_.str(), loc); }} while (0)
#else
#define LOGGER_ERROR_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 50000 
#define LOGGER_FATAL_LOC(log, stream_expr, loc) do {                    \
   if (LOG4CXX_UNLIKELY((log) && (log)->isFatalEnabled())) {            \
    std::ostringstream oss_;                  \
    { oss_ << stream_expr ; }                                  \
    (log)->forcedLog(logging::Level::getFatal(), oss_.str(), loc); }} while (0)
#else
#define LOGGER_FATAL_LOC(logger, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOGGER_DEBUG_PLACE_LOC(log, place, stream_expr, loc) do {      \
  if (LOG4CXX_UNLIKELY((log) && this->log_params().place \
                      && (log)->isDebugEnabled())) {   \
    std::ostringstream oss_;       \
    { oss_ << stream_expr ; }                  \
    (log)->forcedLog(logging::Level::getDebug(), oss_.str(), loc); \
  } \
} while (0)
#define LOG_DEBUG_STATIC_PLACE_LOC(log, place, stream_expr, loc) do {      \
 if (LOG4CXX_UNLIKELY(LogParams::place                  \
               && log::isDebugEnabled())) {      \
    std::ostringstream oss_;       \
    { oss_ << stream_expr ; }                  \
    log::s_logger()->forcedLog(logging::Level::getDebug(), oss_.str(), loc); \
  } \
} while (0)
#else
#define LOGGER_DEBUG_PLACE_LOC(log, place, message, loc)
#define LOG_DEBUG_STATIC_PLACE_LOC(log, place, message, loc)
#endif

#define LOG_DEBUG_PLACE_LOC(log, place, message, loc)  \
  LOGGER_DEBUG_PLACE_LOC(log::s_logger(), place, message, loc)
//#define LOG_DEBUG_STATIC_PLACE_LOC(log, place, message, loc)      LOGGER_DEBUG_STATIC_PLACE_LOC(log::s_logger(), place, message, loc)

#define LOG_DEBUG_PLACE(log, place, message)          \
  LOG_DEBUG_PLACE_LOC(log, place, message, LOG4CXX_LOCATION)
#define LOG_DEBUG_STATIC_PLACE(log, place, message)          \
  LOG_DEBUG_STATIC_PLACE_LOC(log, place, message, LOG4CXX_LOCATION)
#define LOGGER_DEBUG_PLACE(log, place, message)          \
  LOGGER_DEBUG_PLACE_LOC(log, place, message, LOG4CXX_LOCATION)

#define LOG_TRACE_LOC(log, message, loc)     \
  LOGGER_TRACE_LOC(log::s_logger(), message, loc)
#define LOG_DEBUG_LOC(log, message, loc)     \
  LOGGER_DEBUG_LOC(log::s_logger(), message, loc)
#define LOG_INFO_LOC(log, message, loc)      \
  LOGGER_INFO_LOC(log::s_logger(), message, loc)
#define LOG_WARN_LOC(log, message, loc)      \
  LOGGER_WARN_LOC(log::s_logger(), message, loc)
#define LOG_ERROR_LOC(log, message, loc)     \
  LOGGER_ERROR_LOC(log::s_logger(), message, loc)
#define LOG_FATAL_LOC(log, message, loc)     \
  LOGGER_FATAL_LOC(log::s_logger(), message, loc)

#define LOGGER_TRACE(log, message) \
  LOGGER_TRACE_LOC(log, message, \
                   LOG4CXX_LOCATION)
#define LOGGER_DEBUG(log, message) \
  LOGGER_DEBUG_LOC(log, message, \
                   LOG4CXX_LOCATION)
#define LOGGER_INFO(log, message) \
  LOGGER_INFO_LOC(log, message, \
                   LOG4CXX_LOCATION)
#define LOGGER_WARN(log, message) \
  LOGGER_WARN_LOC(log, message, \
                   LOG4CXX_LOCATION)
#define LOGGER_ERROR(log, message) \
  LOGGER_ERROR_LOC(log, message, \
                   LOG4CXX_LOCATION)
#define LOGGER_FATAL(log, message) \
  LOGGER_FATAL_LOC(log, message, \
                   LOG4CXX_LOCATION)

#define LOG_TRACE(log, message) \
  LOGGER_TRACE_LOC(log::s_logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_DEBUG(log, message) \
  LOGGER_DEBUG_LOC(log::s_logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_INFO(log, message) \
  LOGGER_INFO_LOC(log::s_logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_WARN(log, message) \
  LOGGER_WARN_LOC(log::s_logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_ERROR(log, message) \
  LOGGER_ERROR_LOC(log::s_logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_FATAL(log, message) \
  LOGGER_FATAL_LOC(log::s_logger(), message, \
                   LOG4CXX_LOCATION)

// the aliases
#define LOGGER_WARNING LOGGER_WARN
#define LOGGER_WARNING_LOC LOGGER_WARN
#define LOG_WARNING LOG_WARN
#define LOG_WARNING_LOC LOG_WARN

//! Add this to a class declaration for implement logging
//! in the class
#define DEFAULT_LOGGER(class_) \
public: \
  logging::LoggerPtr logger() const override \
  { \
   return log::s_logger(); \
  } \
private: \
  typedef curr::Logger<class_> log;

namespace curr {

/**
 * @defgroup logging
 *
 * Just logging.
 * @{
 */

/**
 * An abstract base for the Logging and Log classes.
 */
class LogBase
{
public:
  /// Create the logger. name == "" means the root logger.
  LogBase (const std::string& name);
  virtual ~LogBase () {}

  std::string GetName() const { return m_sName; }

  /// Pointer to a logger
  logging::LoggerPtr logger;
protected:
  /// Name for the class logger
  std::string m_sName;

  static bool& root_logger_created()
  {
   static bool created = false;
   return created;
  }

private:
  /// Configure the logging system. It is called only once
  /// from the first LogBase constructor call.
  static void Init();
};

inline logging::LoggerPtr GetLogger
(const char * subname, const LogBase& parent) 
{
  return logging::Logger::getLogger
    (parent.GetName() + "." + subname);
}

inline logging::LoggerPtr GetLogger
(const char * subname, const logging::LoggerPtr& parent)
{
  const std::string s = parent->getName();
  return logging::Logger::getLogger(s + "." + subname);
}

class LOG 
{ 
public:
  class Empty {}; // the special val means "no log"
  class Root {}; 
  class Concurrency {};
  class States {};
  class Events {};
  class Connections {};
  class Singletons {};
};

namespace logging {

template<class Type>
struct logger_name
{
  static std::string name()
  {
    using namespace std;
    string s = ::types::type<Type>::name();
    size_t start = 0;
    while((start = s.find("::", start)) != string::npos) 
    {
      s.replace(start, 2, ".");
      ++start;
    }
    return s;
  }
};

// a special logger name "" is used
template<>
struct logger_name<LOG::Root>
{
  static std::string name()
  {
    return std::string();
  }
};

template<class Type>
struct is_root
{
  static constexpr bool value = false;
};

template<>
struct is_root<LOG::Root>
{
  static constexpr bool value = true;
};

}

/**
 */
template<class Type>
class Logger final 
  : public SAutoSingleton<Logger<Type>, false>
{
public:
  Logger() 
    : log_base(
        new LogBase(logging::logger_name<Type>::name())
      )
  {
    this->complete_construction();
    initialized = true;
    LOG_TRACE(
      Logger, 
      "New logger [" << log_base->GetName() << "] is created"
    );
  }

  logging::LoggerPtr logger() const override
  {
    return log_base->logger;
  }

  static logging::LoggerPtr s_logger()
  {
    // These checks are for prevent recursive logging when
    // the log system initialize itself. NB we use an atomic
    // flag, not a state of Existent because its states are
    // functions that call StateMap->Repository->Logging
    // (recursion). But after the root logger
    // initialization all Existent states are definitely
    // constructed. 
    if (Logger<LOG::Root>::is_initialized())
    {
      // this is a recursive call protector, it no guards
      // access from different threads (instance() is
      // reenterable), so this check is no need to be atomic.
      if (state_is(
            Logger::s_the_class(), 
            Logger::TheClass::preinc_exist_oneFun()
          )
      )
        // it is constructing, return the root logger till
        // exist_one state
        return Logger<LOG::Root>::instance()->logger();
      else
        // asks construct the new entity or uses the
        // existing one
        return SAutoSingleton<Logger>::instance()->logger();
    }
    else
      return nullptr;
  }

  static bool isDebugEnabled()
  {
    const auto logger_ptr = s_logger();
    return logger_ptr != nullptr 
      && logger_ptr->isDebugEnabled();
  }

  static bool is_initialized()
  {
    return initialized;
  }

private:
  std::unique_ptr<LogBase> log_base;
  static std::atomic<bool> initialized;
};

template<class Type>
std::atomic<bool> Logger<Type>::initialized(false);

template<>
class Logger<LOG::Empty> final 
  : public SAutoSingleton<Logger<LOG::Empty>>
{
public:
  Logger() 
  {
    this->complete_construction();
  }

  logging::LoggerPtr logger() const override
  {
    return nullptr;
  }

  static logging::LoggerPtr s_logger()
  {
    return nullptr;
  }

  static bool isDebugEnabled()
  {
    return false;
  }
};

namespace {

//! The logging system initializer. If this header is
//! encluded before other the logging system will be
//! initialized before other systems (and deinitialized
//! after all).
logging::LoggerPtr rootLogger = 
  Logger<LOG::Root>::instance()->logger();

}

}

//! @}

#endif
