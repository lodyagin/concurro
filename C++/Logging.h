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

#ifndef CONCURRO_LOGGING_H_
#define CONCURRO_LOGGING_H_
//#pragma warning(disable: 4250 4251)

#include "ObjectWithLogging.h"
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/spi/location/locationinfo.h>
//#include "SSingleton.h"
#include <string>
#include <typeinfo>

namespace curr {

/**
 * @defgroup logging
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

  //log4cxx::LoggerPtr GetLogger () { return logger; }

  /// Pointer to a log4cxx logger
  log4cxx::LoggerPtr logger;
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

inline log4cxx::LoggerPtr GetLogger
(const char * subname, const LogBase& parent) 
{
  return log4cxx::Logger::getLogger
	 (parent.GetName() + "." + subname);
}

inline log4cxx::LoggerPtr GetLogger
(const char * subname, const log4cxx::LoggerPtr& parent)
{
  std::string s;
  parent->getName(s);
  return log4cxx::Logger::getLogger(s + "." + subname);
}

class LOG 
{ 
public:
  class Root {};
  class Thread {};
  class Concurrency {};
  class States {};
  class Events {};
  class Connections {};
};

/**
 */
template<class Type>
class Logger
//: //public LogBase,
//	 public SAutoSingleton< Logger<Type> >
{
public:
  //Logger () : LogBase (typeid(Type).name ()) {}

  static log4cxx::LoggerPtr logger() {
	 /*if (!root_logger_created ()) 
		// TODO make a pre-logger to log before the log
		// system is initialized. Now core dumps.
		return 0;*/

	 if (!base)
		base = init_base (typeid(Type).name ());

	 /*return SAutoSingleton< Logger<Type> >::instance()
		. GetLogger ();*/
	 return base->logger;
  }
protected:
  /// It is used to prior-static-initialization
  /// logging. We hope all data members are initialized
  /// with 0 (http://stackoverflow.com/a/60707/1326885)
  /// prior other initialization. So Logger::logger() will
  /// check if this is null it will create the LogBase
  /// class which is a delegate. 
  static LogBase* base;
  /// It is for partial specialization and use special
  /// names for common log classes (the members of LOG).
  static LogBase* init_base (const std::string& name) {
	 return new LogBase(name);
  }
};

template<class Type>
LogBase* Logger<Type>::base = 0;

template<>
inline LogBase* Logger<LOG::Root>::init_base 
(const std::string& name)
{
  return new LogBase ("");
}

template<>
inline LogBase* Logger<LOG::Thread>::init_base 
(const std::string& name)
{
  return new LogBase ("Thread");
}

template<>
inline LogBase* Logger<LOG::Concurrency>::init_base 
(const std::string& name)
{
  return new LogBase ("Concurrency");
}

template<>
inline LogBase* Logger<LOG::States>::init_base 
(const std::string& name)
{
  return new LogBase ("States");
}

template<>
inline LogBase* Logger<LOG::Events>::init_base 
(const std::string& name)
{
  return new LogBase ("Events");
}

template<>
inline LogBase* Logger<LOG::Connections>::init_base 
(const std::string& name)
{
  return new LogBase ("Connections");
}

// Define a custom log macros for put streams into log
#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOGGER_DEBUG_LOC(log, stream_expr, loc) do {										\
	 if (LOG4CXX_UNLIKELY((log)->isDebugEnabled())) {							\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		(log)->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_), loc); }} while (0)
#else
#define LOGGER_DEBUG_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 5000 
#define LOGGER_TRACE_LOC(log, stream_expr, loc) do {										\
	 if (LOG4CXX_UNLIKELY((log)->isTraceEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		(log)->forcedLog(::log4cxx::Level::getTrace(), oss_.str(oss_), loc); }} while (0)
#else
#define LOGGER_TRACE_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 20000 
#define LOGGER_INFO_LOC(log, stream_expr, loc) do {											\
	 if (LOG4CXX_UNLIKELY((log)->isInfoEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		(log)->forcedLog(::log4cxx::Level::getInfo(), oss_.str(oss_), loc); }} while (0)
#else
#define LOGGER_INFO_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 30000 
#define LOGGER_WARN_LOC(log, stream_expr, loc) do {											\
	 if (LOG4CXX_UNLIKELY((log)->isWarnEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		(log)->forcedLog(::log4cxx::Level::getWarn(), oss_.str(oss_), loc); }} while (0)
#else
#define LOGGER_WARN_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 40000 
#define LOGGER_ERROR_LOC(log, stream_expr, loc) do {										\
	 if (LOG4CXX_UNLIKELY((log)->isErrorEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		(log)->forcedLog(::log4cxx::Level::getError(), oss_.str(oss_), loc); }} while (0)
#else
#define LOGGER_ERROR_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 50000 
#define LOGGER_FATAL_LOC(log, stream_expr, loc) do {										\
	 if (LOG4CXX_UNLIKELY((log)->isFatalEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		(log)->forcedLog(::log4cxx::Level::getFatal(), oss_.str(oss_), loc); }} while (0)
#else
#define LOGGER_FATAL_LOC(logger, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOGGER_DEBUG_PLACE_LOC(log, place, stream_expr, loc) do {			\
  if (LOG4CXX_UNLIKELY(log_params.place \
	                    && (log)->isDebugEnabled())) { 	\
		::log4cxx::helpers::MessageBuffer oss_;		 	\
		{ oss_ << stream_expr ; }							  	\
		(log)->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_), loc); \
  } \
} while (0)
#define LOG_DEBUG_STATIC_PLACE_LOC(log, place, stream_expr, loc) do {			\
 if (LOG4CXX_UNLIKELY(LogParams::place									\
							 && log::logger()->isDebugEnabled())) {			\
		::log4cxx::helpers::MessageBuffer oss_;		 	\
		{ oss_ << stream_expr ; }							  	\
		log::logger()->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_), loc); \
  } \
} while (0)
#else
#define LOGGER_DEBUG_PLACE_LOC(log, place, message, loc)
#define LOG_DEBUG_STATIC_PLACE_LOC(log, place, message, loc)
#endif

#define LOG_DEBUG_PLACE_LOC(log, place, message, loc)	\
  LOGGER_DEBUG_PLACE_LOC(log::logger(), place, message, loc)
//#define LOG_DEBUG_STATIC_PLACE_LOC(log, place, message, loc)		  LOGGER_DEBUG_STATIC_PLACE_LOC(log::logger(), place, message, loc)

#define LOG_DEBUG_PLACE(log, place, message)					\
  LOG_DEBUG_PLACE_LOC(log, place, message, LOG4CXX_LOCATION)
#define LOG_DEBUG_STATIC_PLACE(log, place, message)					\
  LOG_DEBUG_STATIC_PLACE_LOC(log, place, message, LOG4CXX_LOCATION)
#define LOGGER_DEBUG_PLACE(log, place, message)					\
  LOGGER_DEBUG_PLACE_LOC(log, place, message, LOG4CXX_LOCATION)

#define LOG_TRACE_LOC(log, message, loc)		 \
  LOGGER_TRACE_LOC(log::logger(), message, loc)
#define LOG_DEBUG_LOC(log, message, loc)		 \
  LOGGER_DEBUG_LOC(log::logger(), message, loc)
#define LOG_INFO_LOC(log, message, loc)			\
  LOGGER_INFO_LOC(log::logger(), message, loc)
#define LOG_WARN_LOC(log, message, loc)			\
  LOGGER_WARN_LOC(log::logger(), message, loc)
#define LOG_ERROR_LOC(log, message, loc)		 \
  LOGGER_ERROR_LOC(log::logger(), message, loc)
#define LOG_FATAL_LOC(log, message, loc)		 \
  LOGGER_FATAL_LOC(log::logger(), message, loc)

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
  LOGGER_TRACE_LOC(log::logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_DEBUG(log, message) \
  LOGGER_DEBUG_LOC(log::logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_INFO(log, message) \
  LOGGER_INFO_LOC(log::logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_WARN(log, message) \
  LOGGER_WARN_LOC(log::logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_ERROR(log, message) \
  LOGGER_ERROR_LOC(log::logger(), message, \
                   LOG4CXX_LOCATION)
#define LOG_FATAL(log, message) \
  LOGGER_FATAL_LOC(log::logger(), message, \
                   LOG4CXX_LOCATION)

// the aliases
#define LOGGER_WARNING LOGGER_WARN
#define LOGGER_WARNING_LOC LOGGER_WARN
#define LOG_WARNING LOG_WARN
#define LOG_WARNING_LOC LOG_WARN

//! Add this to a class declaration for implement logging
//! in the class
#define DEFAULT_LOGGER(class_) \
private: \
  typedef curr::Logger<class_> log;               \
  \
  log4cxx::LoggerPtr logger() const override \
  { \
	 return log::logger(); \
  }

}

//! @}

#endif
