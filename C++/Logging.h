// -*-Coding: mule-utf-8-unix; fill-column: 58 -*-

#ifndef LOGGING_H_
#define LOGGING_H_
//#pragma warning(disable: 4250 4251)

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
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

  log4cxx::LoggerPtr GetLogger () { return logger; }

protected:
  /// Name for the class logger
  std::string m_sName;
  /// Pointer to a log4cxx logger
  log4cxx::LoggerPtr logger;

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

#if 0
/**
 * This logging class is for embedding in classes and
 * functions.
 */
class Log : public LogBase
{
public:
  /// Constructor for using as class logger
  Log (const std::string& szName);
  /// Constructor for using in functions
  Log (const std::string& szName, 
		 const LogBase& ParentLogger);
  virtual ~Log ();
	
protected:
  /// true if the class used as AutoLogger
  bool m_bFunctionLogger;
};
#endif

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

/*class ClassWithLogging
{
public:
  const std:string _logging_class_name;
protected:
  ClassWithLogging (const std::string& className)
	 : _logging_class_name (className);
	 };*/

/**
 */
template<class Type>
class Logger
  : public LogBase,
	 public SAutoSingleton< Logger<Type> >
{
public:
  Logger () : LogBase (typeid(Type).name ()) {}

  static log4cxx::LoggerPtr logger() {
	 if (!root_logger_created ()) 
		// TODO make a pre-logger to log before the log
		// system is initialized. Now core dumps.
		return 0;

	 return SAutoSingleton< Logger<Type> >::instance()
		. GetLogger();
  }
};

template<>
inline Logger<LOG::Root>::Logger() : LogBase("") {}

template<>
inline Logger<LOG::Thread>::Logger() : LogBase("Thread") {}

template<>
inline Logger<LOG::Concurrency>::Logger() 
: LogBase("Concurrency"){}

template<>
inline Logger<LOG::States>::Logger() : LogBase("States") {}

// Define a custom log macros for put streams into log
#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOG4STRM_DEBUG(log, stream_expr) { \
	 if (LOG4CXX_UNLIKELY(log::logger()->isDebugEnabled())) {				\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_), LOG4CXX_LOCATION); }} while (0)
#else
#define LOG4STRM_DEBUG(log, message)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 5000 
#define LOG4STRM_TRACE(log, stream_expr) {									\
	 if (LOG4CXX_UNLIKELY(log::logger()->isTraceEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getTrace(), oss_.str(oss_), LOG4CXX_LOCATION); }} while (0)
#else
#define LOG4STRM_TRACE(log, message)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 20000 
#define LOG4STRM_INFO(log, stream_expr) {										\
	 if (LOG4CXX_UNLIKELY(log::logger()->isInfoEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getInfo(), oss_.str(oss_), LOG4CXX_LOCATION); }} while (0)
#else
#define LOG4STRM_INFO(log, message)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 30000 
#define LOG4STRM_WARN(log, stream_expr) {										\
	 if (LOG4CXX_UNLIKELY(log::logger()->isWarnEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getWarn(), oss_.str(oss_), LOG4CXX_LOCATION); }} while (0)
#else
#define LOG4STRM_WARN(log, message)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 40000 
#define LOG4STRM_ERROR(log, stream_expr) {									\
	 if (LOG4CXX_UNLIKELY(log::logger()->isErrorEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getError(), oss_.str(oss_), LOG4CXX_LOCATION); }} while (0)
#else
#define LOG4STRM_ERROR(log, message)
#endif

#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 50000 
#define LOG4STRM_FATAL(log, stream_expr) {									\
	 if (LOG4CXX_UNLIKELY(log::logger()->isFatalEnabled())) {						\
		::log4cxx::helpers::MessageBuffer oss_;									\
		{ oss_ << stream_expr ; }																	\
		log::logger()->forcedLog(::log4cxx::Level::getFatal(), oss_.str(oss_), LOG4CXX_LOCATION); }} while (0)
#else
#define LOG4STRM_FATAL(logger, message)
#endif

#endif
