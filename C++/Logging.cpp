#include "StdAfx.h"
#ifndef _WIN32
#  include "AppConfig.h"
#else
#  include <sys/types.h>
#  include <unistd.h>
#endif
#include "Logging.h"
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/properties.h>
#include <fstream>


bool Logging::m_bConfigured = false;

log4cxx::LoggerPtr Logging::m_RootLogger
  (log4cxx::Logger::getRootLogger());
log4cxx::LoggerPtr Logging::m_ThreadLogger
  (log4cxx::Logger::getLogger("Thread"));
log4cxx::LoggerPtr Logging::m_ConcurrencyLogger
  (log4cxx::Logger::getLogger("Concurrency"));
log4cxx::LoggerPtr Logging::m_StatesLogger
  (log4cxx::Logger::getLogger("States"));

Logging::Logging(const char* szName)
   : m_pLogger(0)
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
#ifdef _WIN32
      HMODULE hModule = GetModuleHandle(NULL);
      WCHAR szFilename[MAX_PATH] = L"";
      GetModuleFileName(hModule,szFilename,MAX_PATH); //FIXME path overflow
      std::wstring sFilename = szFilename;
#  if 0
      {
         std::ofstream a ("log.log");
         a << szFilename << std::endl;
      }
#  endif
      const std::wstring::size_type idx = sFilename.rfind(L'\\');
      std::wstring sModuleDir = sFilename.substr(0,idx);

		SetCurrentDirectory(sModuleDir.c_str());

		std::wstring fileName;
		fileName = sModuleDir + L"\\log4cxx.properties";
#else
		const std::string& fileName = std::string("/etc/") 
		  + APPCONFIG_PKG_NAME + "/log4cxx.properties";
#endif

		// put our PID into env
#ifdef _WIN32
		::_wputenv (SFORMAT(L"LOGPID="<<GetCurrentProcessId()).c_str()); 
#else
		::putenv (string2char_ptr (SFORMAT("LOGPID="<<getpid()))); 
#endif

		{ 
		  std::ifstream f(fileName.c_str()); 
		  if ((void*)f == 0) throw 0;
		}
		log4cxx::PropertyConfigurator::configure (fileName);
   }
   catch (...) 
   {	// here let's do default configuration
	   log4cxx::helpers::Properties p;
#ifndef _DEBUG
	   p.setProperty(_T"log4j.rootLogger", _T"INFO, A1");
	   p.setProperty(_T"log4j.appender.A1", _T"org.apache.log4j.RollingFileAppender");
	   p.setProperty(_T"log4j.appender.A1.Append", _T"True");
	   p.setProperty(_T"log4j.appender.A1.File", _T"coressh.log");
	   p.setProperty(_T"log4j.appender.A1.MaxFileSize", _T"1048576");
	   p.setProperty(_T"log4j.appender.A1.MaxBackupIndex", _T"12");
	   p.setProperty(_T"log4j.appender.A1.layout", _T"org.apache.log4j.PatternLayout");
	   p.setProperty(_T"log4j.appender.A1.layout.ConversionPattern", _T"%p %d{%Y-%m-%d %H:%M:%S} %t %m%n");
#else
	   p.setProperty(_T"log4j.rootLogger", _T"DEBUG, A1");
	   p.setProperty(_T"log4j.appender.A1", _T"org.apache.log4j.ConsoleAppender");
	   p.setProperty(_T"log4j.appender.A1.layout", _T"org.apache.log4j.PatternLayout");
	   p.setProperty
       (_T"log4j.appender.A1.layout.ConversionPattern", 
        _T"%p %d{%Y-%m-%d %H:%M:%S} %t %C.%M%n----%n%m%n%n");
#endif
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

static int zqw = (Logging::Init(), 1); // as Stroustrup teaches :>

