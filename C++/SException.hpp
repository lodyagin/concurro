/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
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

#ifndef CONCURRO_SEXCEPTION_HPP
#define CONCURRO_SEXCEPTION_HPP

#include "SException.h"
#include "Logging.h"

//! Throw the stored exception and log the occurence place.
template<class Log = curr::Logger<curr::LOG::Root>>
void log_and_throw [[ noreturn ]]
  (std::exception_ptr excp,
   log4cxx::spi::LocationInfo&& loc =  LOG4CXX_LOCATION,
   log4cxx::LoggerPtr l = Log::logger())
{
  try {
    std::rethrow_exception(excp);
  }
  catch (const SException& e2) {
    LOGGER_DEBUG_LOC(l, "Throw exception " << e2, loc);
    throw;
  }
}

#define THROW_EXCEPTION(exception_class, par...) \
do { \
  curr::log_and_throw( \
     std::make_exception_ptr(exception_class{par}), \
       LOG4CXX_LOCATION);  \
} while (0)

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

#endif

