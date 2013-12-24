/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU Lesser General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public License
  for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#include "StdAfx.h"
#ifndef _WIN32
//#  include "AppConfig.h"
// FIXME
#define APPCONFIG_PKG_NAME "project"
#else
#  include <sys/types.h>
#  include <unistd.h>
#endif
#include <fstream>
#include <iostream>
#include <unistd.h>
#include "Logging.h"
#include "SCheck.h"
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/properties.h>

namespace curr {

LogBase::LogBase (const std::string& szName)
  : m_sName (szName)/*,
							 logger (log4cxx::Logger::getLogger(szName.c_str()))*/
{
  if (!szName.empty ()) {
	 LOG_TRACE(Logger<LOG::Root>,
		"Create the [" << szName << "] logger.");
  }
  else {
	 // only one root logger can be created
	 SCHECK (!root_logger_created());
	 root_logger_created() = true;
	 // the check above guarantees Init will call no more
	 // than once
	 Init ();
  }
  logger = log4cxx::Logger::getLogger(szName.c_str());
}

void LogBase::Init()
{
  try 
  {
#ifdef _WIN32
	 HMODULE hModule = GetModuleHandle(NULL);
	 WCHAR szFilename[MAX_PATH] = L"";
	 //FIXME path overflow
	 GetModuleFileName(hModule,szFilename,MAX_PATH); 
	 std::wstring sFilename = szFilename;
#  if 0
	 {
		std::ofstream a ("log.log");
		a << szFilename << std::endl;
	 }
#  endif
	 const std::wstring::size_type idx = 
		sFilename.rfind (L'\\');
	 std::wstring sModuleDir = sFilename.substr(0,idx);

	 SetCurrentDirectory(sModuleDir.c_str());

	 std::wstring fileName;
	 fileName = sModuleDir + L"\\log4cxx.properties";
#else


	 const std::string& fileName =
		 std::ifstream("log4cxx.properties").good() ? "log4cxx.properties"
			 :std::string("/etc/") + APPCONFIG_PKG_NAME + "/log4cxx.properties";

#endif

	 // put our PID into env
#ifdef _WIN32
	 ::_wputenv(SFORMAT(L"LOGPID="
							  <<GetCurrentProcessId()).c_str());
#else
	 ::putenv(string2char_ptr(SFORMAT("LOGPID="
												 << getpid()))); 
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
	 p.setProperty(_T"log4j.appender.A1.File", 
						APPCONFIG_PKG_NAME ".log");
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

#if 0
Log::Log (const std::string& szName)
  : LogBase(szName),
    m_bFunctionLogger(false)
{}

Log::Log (const std::string& szName, 
			 const LogBase& ParentLogger)
  : LogBase (ParentLogger.GetName() + "." + szName),
	 m_bFunctionLogger(true)
{}

Log::~Log () {}

#endif

}
