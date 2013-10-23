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

#ifndef CONCURRO_SEXCEPTION_H_
#define CONCURRO_SEXCEPTION_H_

#include "SCommon.h"
#include "HasStringView.h"
#include "Logging.h"
#include <exception>
#include <ostream>
#include <log4cxx/spi/location/locationinfo.h>

namespace curr {

//! A base for the all concurro exceptions
class SException 
  : public std::exception, public HasStringView
{
public:

  explicit SException (const std::string & what/*, bool alreadyLogged = false*/);
  explicit SException (const std::wstring & what/*, bool alreadyLogged = false*/);
  virtual ~SException() throw();

  bool isAlreadyLogged () const  { return alreadyLoggedFlag; }

  virtual const char * what() const throw();

  void outString(std::ostream& out) const
  {
	 out << this->what();
  }

protected:
#ifdef _WIN32
  std::wstring whatU;
#endif
  std::string _what;
  bool alreadyLoggedFlag;
};

//! A type passed for functions/methods.
//! It keeps the location and the logger of the caller. 
struct ThrowSException 
{
  ThrowSException
    (log4cxx::LoggerPtr l,
     const log4cxx::spi::LocationInfo& loc = 
      LOG4CXX_LOCATION)
    : debug_location(loc), logger(l)
  {}

  template<class Exception>
  void raise(Exception&& exc) const
  {
    LOGGER_INFO_LOC(logger, "Throw exception " << exc, 
                  debug_location);
    throw exc;
  }

  const log4cxx::spi::LocationInfo debug_location;
  const log4cxx::LoggerPtr logger;
};

//! Throw the exception and log the occurence place.
template<class Log = curr::Logger<curr::LOG::Root>>
void log_and_throw [[ noreturn ]]
  (const SException& excp,
   log4cxx::LoggerPtr l = Log::logger(),
   log4cxx::spi::LocationInfo&& loc =  LOG4CXX_LOCATION)
{
  LOGGER_DEBUG_LOC(l, "Throw exception " << excp, loc);
  throw excp;
}

//! Throw the stored exception and log the occurence place.
template<class Log = curr::Logger<curr::LOG::Root>>
void log_and_throw [[ noreturn ]]
  (std::exception_ptr excp,
   log4cxx::LoggerPtr l = Log::logger(),
   log4cxx::spi::LocationInfo&& loc =  LOG4CXX_LOCATION)
{
  try {
    std::rethrow_exception(excp);
  }
  catch (const SException& e2) {
    LOGGER_DEBUG_LOC(l, "Throw exception " << e2, loc);
    throw excp;
  }
}

//! Exception: Program Error (means general logic error)
class ProgramError : public SException
{
public:
  ProgramError() : SException("Program Error") {}
};

//! Exception: Not implemented
class NotImplemented : public SException
{
public:
  NotImplemented() : SException("Not implemented") {}
};

//! @deprecated Use log_and_throw() instead
#define THROW_EXCEPTION(exception_class, par...) \
do { \
  log_and_throw(exception_class{par}); \
} while (0)

//! @deprecated Use log_and_throw() instead
#define THROW_EXCEPTION_PLACE(place, exception_class, par...) do { \
	 exception_class exc_{par};			 \
  LOG_DEBUG_PLACE(curr::Logger<curr::LOG::Root>, place,   \
    "Throw exception " << exc_); \
  throw exc_; \
  } while (0)

#define THROW_PROGRAM_ERROR \
  THROW_EXCEPTION(curr::ProgramError)

#define THROW_NOT_IMPLEMENTED \
  THROW_EXCEPTION(curr::NotImplemented)

// user mistake - wrong action, invalid configuration etc
class SUserError : public SException
{
public:

  typedef SException Parent;

  SUserError( const std::string & what ) : Parent(what) {}
  SUserError( const std::wstring & what ) : Parent(what) {}

};


SMAKE_THROW_FN_DECL(sUserError, SUserError)

std::ostream& operator<< (std::ostream&, const SException& exc);

// This macro is disabled because it hides exceptions from doxygen
#if 0
#define DEFINE_EXCEPTION(class_, msg) \
class class_ : public curr::SException \
{ \
public: \
  class_() : curr::SException(msg) {} \
};
#endif

class BadCastBase : public SException
{
public:
  BadCastBase(const std::string& s) : SException(s) {}
};

//! Exception: bad cast
template<class Target, class Source>
class BadCast : public BadCastBase
{
public:
  BadCast(const Source& src)
    : BadCastBase
      (SFORMAT("Bad cast of a value of type " 
         << typeid(Source).name()
         << "to " << typeid(Target).name())),
      source(src)
  {}

  //! A source value
  const Source source;
};

}
#endif
