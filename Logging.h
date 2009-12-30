#pragma once
//#pragma warning(disable: 4250 4251)

#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <string>

class Logging
{
private:
   // true if Init() was called
   // otherwise false
   static bool m_bConfigured;

   static log4cxx::LoggerPtr m_RootLogger;
   static log4cxx::LoggerPtr m_ThreadLogger;
   static log4cxx::LoggerPtr m_TrackLogger;
   static log4cxx::LoggerPtr m_ConcurrencyLogger;

   // Pointer to logger
   log4cxx::LoggerPtr m_pLogger;
   // Name for class logger
   std::string m_sName;
   // true if class used as AutoLogger
   bool m_bFunctionLogger;
public:
   // Constructor for class logger
   Logging(const char* szName);
   // Constructor for function logger
   Logging(const char* szName, const Logging& ParentLogger);
   ~Logging();

   inline log4cxx::LoggerPtr GetLogger()
   {
      if (!m_bConfigured)
         Init();

      if (m_pLogger == NULL)
      {
         m_pLogger = log4cxx::Logger::getLogger(m_sName.c_str());
      }
      return m_pLogger;
   }
   inline std::string GetName() const { return m_sName; }

   // Reads settings for logger
   static void Init();

   static inline log4cxx::LoggerPtr Root() { return m_RootLogger ; };
   static inline log4cxx::LoggerPtr Thread() { return m_ThreadLogger ; };
   static inline log4cxx::LoggerPtr Track() { return m_TrackLogger ; };
   static inline log4cxx::LoggerPtr Concurrency() { return m_ConcurrencyLogger ; };
};

inline log4cxx::LoggerPtr GetLogger(const char * subname, const Logging& parent) {
	return log4cxx::Logger::getLogger(parent.GetName() + "." + subname);
}

inline log4cxx::LoggerPtr GetLogger(const char * subname, const log4cxx::LoggerPtr& parent) {
	std::string s;
	parent->getName(s);
	return log4cxx::Logger::getLogger(s + "." + subname);
}

static int zqw = (Logging::Init(), 1); // as teaches Stroustrup :>

// Define a custom log macros for put streams into log
#if !defined(LOG4CXX_THRESHOLD) || LOG4CXX_THRESHOLD <= 10000 
#define LOG4STRM_DEBUG(logger, stream_expr) { \
        if (LOG4CXX_UNLIKELY((logger)->isDebugEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           { stream_expr ; } \
           (logger)->forcedLog(::log4cxx::Level::getDebug(), oss_.str(oss_), LOG4CXX_LOCATION); }}
#else
#define LOG4CXX_DEBUG(logger, message)
#endif
