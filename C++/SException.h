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
#include <exception>
#include <ostream>

namespace curr {

// base for the all SCommon exceptions

class SException : public std::exception, public HasStringView
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

#define THROW_EXCEPTION(exception_class, par...) do { \
	 exception_class exc_{par};								\
  LOG_DEBUG(curr::Logger<curr::LOG::Root>, \
    "Throw exception " << exc_); \
  throw exc_; \
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

//! Exception: invalid cast (fromString())
class FromStringCastException
  : public boost::bad_lexical_cast,
    public SException 
{
public:
  FromStringCastException(boost::bad_lexical_cast e) :
    boost::bad_lexical_cast(e.source_type(), e.target_type()),
    SException("FromStringCastException, bad cast"){}
};

}
#endif
