// -*-Coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef LOGGING_H_
#define LOGGING_H_
//#pragma warning(disable: 4250 4251)

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/spi/location/locationinfo.h>
#include "SSingleton.h"
#include <string>
#include <typeinfo>

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

// Define a custom log macros for put streams into log
#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOG_DEBUG_LOC(log, stream_expr, loc) {										\
	 if (LOG4CXX_UNLIKELY(log::logger()->isDebugEnabled())) {				\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_), loc); }} while (0)
#else
#define LOG_DEBUG_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 5000 
#define LOG_TRACE_LOC(log, stream_expr, loc) {										\
	 if (LOG4CXX_UNLIKELY(log::logger()->isTraceEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getTrace(), oss_.str(oss_), loc); }} while (0)
#else
#define LOG_TRACE_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 20000 
#define LOG_INFO_LOC(log, stream_expr, loc) {											\
	 if (LOG4CXX_UNLIKELY(log::logger()->isInfoEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getInfo(), oss_.str(oss_), loc); }} while (0)
#else
#define LOG_INFO_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 30000 
#define LOG_WARN_LOC(log, stream_expr, loc) {											\
	 if (LOG4CXX_UNLIKELY(log::logger()->isWarnEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getWarn(), oss_.str(oss_), loc); }} while (0)
#else
#define LOG_WARN_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 40000 
#define LOG_ERROR_LOC(log, stream_expr, loc) {										\
	 if (LOG4CXX_UNLIKELY(log::logger()->isErrorEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getError(), oss_.str(oss_), loc); }} while (0)
#else
#define LOG_ERROR_LOC(log, message, loc)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 50000 
#define LOG_FATAL_LOC(log, stream_expr, loc) {										\
	 if (LOG4CXX_UNLIKELY(log::logger()->isFatalEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getFatal(), oss_.str(oss_), loc); }} while (0)
#else
#define LOG_FATAL_LOC(logger, message, loc)
#endif

#define LOG_TRACE(logger, message) \
  LOG_TRACE_LOC(logger, message, LOG4CXX_LOCATION)
#define LOG_DEBUG(logger, message) \
  LOG_DEBUG_LOC(logger, message, LOG4CXX_LOCATION)
#define LOG_INFO(logger, message) \
  LOG_INFO_LOC(logger, message, LOG4CXX_LOCATION)
#define LOG_WARN(logger, message) \
  LOG_WARN_LOC(logger, message, LOG4CXX_LOCATION)
#define LOG_ERROR(logger, message) \
  LOG_ERROR_LOC(logger, message, LOG4CXX_LOCATION)
#define LOG_FATAL(logger, message) \
  LOG_FATAL_LOC(logger, message, LOG4CXX_LOCATION)

// the aliases
#define LOG_WARNING LOG_WARN
#define LOG_WARNING_LOC LOG_WARN

#endif