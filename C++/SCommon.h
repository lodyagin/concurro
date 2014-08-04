/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 *
 * @author Sergei Lodyagin
 */

#ifndef SCOMMON_H_
#define SCOMMON_H_

#include <iostream>
#include <sstream>
#include <ostream>
#include <iomanip>
#include <string>
#include <tuple>
#include <stdarg.h>
#ifdef _WIN32
#include <atlbase.h>
#else
#include <errno.h>
#define __declspec(x)
#define __stdcall
#define INVALID_SOCKET (-1)
#define BOOL bool
#define SOCKET int
#endif

#include <boost/lexical_cast.hpp>
#include "types/meta.h"

//#define WIN32_LEAN_AND_MEAN 
//#include <windows.h>

namespace curr {

#ifdef _WIN32
#  define _T L
#else
#  define _T
#endif

// cut out not more than maxCount chars that equal to passed one. If maxCount == -1 then cut out all
std::string trimLeft(const std::string &, char = ' ', int maxCount =
    -1);
std::string trimRight(const std::string &, char = ' ',
    int maxCount = -1);
std::string trimBoth(const std::string &, char = ' ', int maxCount =
    -1);

const char * strnchr(const char * str, int chr, size_t maxLen);
//int strnlen( const char * str, size_t maxLen );  // returns -1 if len > maxLen

/* Copy string src to buffer dest (of buffer size
 * dest_size).  At most dest_size-1 characters will be
 * copied.  Always NUL terminates (unless dest_size == 0).
 * This function does NOT allocate memory.  Unlike
 * strncpy, this function doesn't pad dest (so it's often
 * faster).  Returns size of attempted result,
 * strlen(src), so if retval >= dest_size, truncation
 * occurred.
 */
size_t strlcpy(char *dest, const char *src, size_t dest_size);

/* It is got from glib 2.0.
 *
 * Appends string src to buffer dest (of buffer size
 * dest_size).  At most dest_size-1 characters will be
 * copied.  Unlike strncat, dest_size is the full size of
 * dest, not the space left over.  This function does NOT
 * allocate memory.  This always NUL terminates (unless
 * siz == 0 or there were no NUL characters in the
 * dest_size characters of dest to start with).  Returns
 * size of attempted result, which is MIN (dest_size,
 * strlen (original dest)) + strlen (src), so if retval >=
 * dest_size, truncation occurred.
 */
size_t
strlcat(char *dest, const char *src, size_t dest_size);

#ifdef _WIN32
/**
 * It is got from glib 2.0.
 *
 * @string: the return location for the newly-allocated string.
 * @format: a standard printf() format string, but notice
 *          <link linkend="string-precision">string precision pitfalls</link>.
 * @args: the list of arguments to insert in the output.
 *
 * This function allocates a 
 * string to hold the output, instead of putting the output in a buffer 
 * you allocate in advance.
 *
 * Returns: the number of bytes printed.
 **/
int
vasprintf (char **string,
    char const *format,
    va_list args);
#endif

char *
strsep(char **stringp, const char *delim);

#ifdef _WIN32
// convert windows error code to string
std::wstring sWinErrMsg (DWORD errorCode);
#endif

#if 0
// formats string a la sprintf, max 10k size
std::wstring sFormat(std::wstring format, ...);
#endif

#define WSFORMAT(e) ((dynamic_cast<const std::wostringstream&>(std::wostringstream().flush() << e)).str())
#ifndef UNICODE
#define SFORMAT(e) ((dynamic_cast<const std::ostringstream&>(std::ostringstream().flush() << e)).str())
#else
#define SFORMAT WSFORMAT
#endif

namespace {

#if 0
//! A helper class for sformat()
template<class... Args>
struct sformat_type :: std::touple
{
  using std::touple::touple;
};
#endif

template<class Stream>
struct stream_out
{
  stream_out(Stream& os_) : os(os_) {}

  template<class T>
  void out(T&& v) { os << v; }

  Stream& os;
};

}

template<class... Args>
std::ostream& operator<< 
  ( std::ostream& out,
    std::tuple<Args...>&& parts)
{
  types::for_each(parts);
  return out;
}

#if 0
template<class... Args>
std::string sformat(Args&&... args)
{
  using ::curr::operator<<;
  std::ostringstream oss;
  ::curr::operator<<(oss, std::forward_as_tuple
    (std::forward<Args>(args)...));
  //oss << std::forward_as_tuple
  //  (std::forward<Args>(args)...);
  return oss.str();
}
#else
inline std::ostream& output(std::ostream& out)
{
  return out;
}

template<class Arg0, class... Args>
std::ostream&
output(std::ostream& out, Arg0&& arg0, Args&&... args)
{
  out << std::forward<Arg0>(arg0);
  return output(out, std::forward<Args>(args)...);
}

template<class... Args>
std::string sformat(Args&&... args)
{
  std::ostringstream oss;
  output(oss, std::forward<Args>(args)...);
  return oss.str();
}
#endif

std::string AmountFormat(double amt, int precision = 2);
std::string StripDotZeros(const std::string& s);
std::string RateFormat(double rate);	//2 or 4 fractional digits
std::string FixFormat(double lot, int precision = 1);	//0 or 1 fractional digit

std::wstring sFormatVa(const std::wstring & format, va_list list);

// ansi version
std::string sFormatVaA(const std::string & format, va_list list);

//string uni2ascii (const std::wstring & str);
//std::wstring ascii2uni (const string & str);

#define FORMAT_SYS_ERR(sysFun, sysErr) \
   (SFORMAT("When calling '" << sysFun << " (...)' the system error '" \
   << sWinErrMsg (sysErr) << "' has occured (#" << sysErr << ")."))

#if 0
// throws std::exception, formating string first
__declspec(noreturn)void sThrow(const wchar_t * format, ...);
#endif

// load string with given id from resources. Maximum string length is 10k
std::string loadResourceStr(int id);

#ifdef _WIN32
void checkHR( HRESULT );
#endif

std::wstring str2wstr(const std::string &);
std::string wstr2str(const std::wstring &);

std::string toUTF8(const std::wstring&);
std::wstring fromUTF8(const std::string&);

inline const char * ptr2ptr(const std::string & s) {
  return s.c_str();
}

inline const wchar_t * ptr2ptr(const std::wstring & s) {
  return s.c_str();
}

inline const char * sptr(const char * p) {
  return p ? p : "";
}

inline const char * szptr(const char * p) {
  return p && *p ? p : 0;
}

template<class T>
inline const char * ptr2sptr(const T & s) {
  return sptr(ptr2ptr(s));
}

template<class T>
inline const char * ptr2szptr(const T & s) {
  return szptr(ptr2ptr(s));
}

// append a string representation of an object
// to a string
template<class T>
void toString(const T& object, std::string & s) {
  s = boost::lexical_cast<std::string>(object);
}

template<class T>
std::string toString(const T& object) {
  return boost::lexical_cast<std::string>(object);
}

template<class Target, class Source>
class BadCast;

// FIXME raise exception when the string is not a number
template<class T, class String>
T fromString(String&& s) 
{
  try 
  {
    return boost::lexical_cast<T>(std::forward<String>(s));
  } 
  catch (const boost::bad_lexical_cast&) 
  {
    throw BadCast<T, String>(s);
  }
}

// FIXME raise exception when the string is not a number
template<class T, class CharT>
T fromString(const std::basic_string<CharT>& s) 
{
  try 
  {
    return boost::lexical_cast<T>(s);
  } 
  catch (const boost::bad_lexical_cast&) 
  {
    throw BadCast<T, std::basic_string<CharT>>(s);
  }
}

#define SMAKE_THROW_FN_DECL(name, XClass)  \
void name( const wchar_t * fmt, ... ); void name(const std::wstring& msg);
//void name( const char * fmt, ... ); void name(const std::string& msg);

SMAKE_THROW_FN_DECL(sThrow, SException)

#define SMAKE_THROW_MEMBER_DECL(name, XClass)  \
static void name( const wchar_t * fmt, ... );
//static void name( const char * fmt, ... );

#define SMAKE_THROW_FN_IMPL(name, XClass)  \
  \
void name( const wchar_t * fmt, ... )  \
{  \
  va_list va;  \
  va_start(va, fmt);  \
  std::wstring msg(sFormatVa(fmt, va));  \
  va_end(va);  \
  throw XClass(msg);  \
}; void name(const std::wstring& msg) { throw XClass(msg); };

//template<class X>
//SMAKE_THROW_FN_IMPL(sThrowX, X)

#ifdef _WIN32
FILETIME TimetToFileTime (time_t t);
time_t FileTimeToTimet (FILETIME ft);
#endif

// copy if (see Stroustrup 3rd ed, 18.6.1)

template<class In, class Out, class Pred>
Out copy_if(In first, In last, Out res, Pred p) {
  while (first != last) {
    if (p(*first))
      *res++ = *first;
    ++first;
  }
  return res;
}

/**
 * Allocate a new char* string by the std::string arg.
 */
char* string2char_ptr(const std::string& str);

#if 0
//! Calculate the logic implication: a -> b
#define IMPLICATION(a, b) ((b) || (!(a)))
#else
//! Calculate the logic implication: a -> b
inline bool implication(bool a, bool b)
{
  return b || !a;
}
#endif

/**
 * Abstract wrapper for a class method pointer. It allows
 * has pointer to any method of any class derived from
 * Base. 
 */
template<class Base, class ... Args>
class AbstractMethodCallWrapper {
public:
  virtual void call(Base* obj, Args ... args) = 0;
};

/**
 * An AbstractMethodCallWrapper descendant for holding
 * pointers of class Derived.
 */
template<class Derived, class Base, class ... Args>
class MethodCallWrapper: public AbstractMethodCallWrapper<Base,
    Args...> {
public:
  typedef void (Derived::*Method)(Args ... args);

  MethodCallWrapper(Method a_method) :
      method(a_method) {
  }

  void call(Base* obj, Args ... args) override
  {
    (dynamic_cast<Derived*>(obj)->*method)(args...);
  }
protected:
  Method method;
};

}
#endif
