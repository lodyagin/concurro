#include "stdafx.h"
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/properties.h>
#include <fstream>


bool Logging::m_bConfigured = false;

log4cxx::LoggerPtr Logging::m_RootLogger(log4cxx::Logger::getRootLogger());
log4cxx::LoggerPtr Logging::m_ThreadLogger(log4cxx::Logger::getLogger("Thread"));
log4cxx::LoggerPtr Logging::m_ConcurrencyLogger(log4cxx::Logger::getLogger("Concurrency"));

Logging::Logging(const char* szName)
   : m_pLogger(NULL)
   , m_sName(szName)
   , m_bFunctionLogger(false)
{
}

Logging::Logging(const char* szName, const Logging& ParentLogger)
   : m_bFunctionLogger(true)
{
   m_sName = ParentLogger.GetName();
   m_sName += ".";
   m_sName += szName;
   /*if (GetLogger()->isDebugEnabled())
   {
      char s[100] = "";
      sprintf(s,"[%d][ --== begin ==-- ]");
      LOG4CXX_DEBUG(GetLogger(),s);
   }*/
}

Logging::~Logging()
{
   /*if (m_bFunctionLogger && GetLogger()->isDebugEnabled())
   {
      char s[100] = "";
      sprintf(s,"[%d][ --==  end  ==-- ]");
      LOG4CXX_DEBUG(GetLogger(),s);
   }*/
}


void Logging::Init()
{
   if (m_bConfigured)
      return;
   m_bConfigured = true;

   try 
   {
      HMODULE hModule = GetModuleHandle(NULL);
      char szFilename[MAX_PATH] = "";
      GetModuleFileName(hModule,szFilename,MAX_PATH);
      std::string sFilename = szFilename;
#if 0
      {
         std::ofstream a ("c:\\log.log");
         a << szFilename << std::endl;
      }
#endif
	  const std::string::size_type idx = sFilename.rfind('\\');
      std::string sModuleDir = sFilename.substr(0,idx);

	  SetCurrentDirectory(sModuleDir.c_str());

      std::string fileName;
      fileName = sModuleDir + "\\log4cxx.properties";

	  _putenv (SFORMAT("LOGPID="<<GetCurrentProcessId()).c_str()); // put our PID into env

    { 
      std::ifstream f(fileName.c_str()); 
      if ((void*)f == 0) throw 0;
    }
	  log4cxx::PropertyConfigurator::configure (fileName);
   }
   catch (...) 
   {	// here let's do default configuration
	   log4cxx::helpers::Properties p;
	   p.setProperty(L"log4j.rootLogger", L"INFO, A1");
	   p.setProperty(L"log4j.appender.A1", L"org.apache.log4j.RollingFileAppender");
	   p.setProperty(L"log4j.appender.A1.Append", L"True");
	   p.setProperty(L"log4j.appender.A1.File", L"coressh.log");
	   p.setProperty(L"log4j.appender.A1.MaxFileSize", L"1048576");
	   p.setProperty(L"log4j.appender.A1.MaxBackupIndex", L"12");
	   p.setProperty(L"log4j.appender.A1.layout", L"org.apache.log4j.PatternLayout");
	   p.setProperty(L"log4j.appender.A1.layout.ConversionPattern", L"%p %d{%Y-%m-%d %H:%M:%S} %m%n");
	   /* this is BasicConfigurator properties. 
	   We change DEBUG to WARN, and ConsoleAppender to NTEventLog
	   log4j.rootLogger=DEBUG, A1
	   log4j.appender.A1=org.apache.log4j.ConsoleAppender
	   log4j.appender.A1.layout=org.apache.log4j.PatternLayout 
	   log4j.appender.A1.layout.ConversionPattern=%-4r [%t] %-5p %c %x - %m%n 
	   */
	   log4cxx::PropertyConfigurator::configure(p);
   }
}

static int zqw = (Logging::Init(), 1); // as teaches Stroustrup :>

