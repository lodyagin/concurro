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

#include <ios>
#include <cstddef>
#include <stdexcept>

#ifndef CONCURRO_ENUM_H_
#define CONCURRO_ENUM_H_

namespace curr {

constexpr bool string_equal
  (const char* a, const char* b)
{
  return a[0] == b[0]
    && (*a == 0 || string_equal(a+1, b+1));
}

template<class Int, class String, std::size_t N>
class EnumMeta
{
public:
  constexpr String name(Int k)
  {
    return (k >= 0 && k < N) 
      ? names[k] 
      : (throw std::domain_error("Bad enum value"));
  }

  constexpr Int value(String name)
  {
    return search_value(names, 0, name);
  }

  String names[N];

protected:
  constexpr Int search_value
    (String const* arr, std::size_t k, String name)
  {
    return (k < N) ?
      (string_equal(*arr, name) ? 
        k : search_value(arr + 1, k + 1, name))
      : (throw std::domain_error("Bad enum constant"));
  }
};

template<class V0, class... V/*, class Int = int*/>
constexpr EnumMeta<int, V0, 1 + sizeof...(V)> 
//
BuildEnum(V0 v0, V... v)
{
  return EnumMeta<int, V0, 1 + sizeof...(V)> { v0, v... };
}

class EnumBase
{
public:
  static const int xalloc;
};

/**
  * An alternative to enum. LiteralType
  * TODO const char (&) [N] and check array-to-pointer
  * rules in the standard
  */
template <
  class T, 
  class Int = int,
  class String = const char*
>
class Enum : public EnumBase
{
public:
  constexpr Enum() : value(0)
  {
  }

  constexpr Enum(String name) : value(T::meta.value(name))
  {
  }

  constexpr String name()
  {
    return T::meta.name(value);
  }

  constexpr bool operator == (Enum b)
  {
    return value == b.value;
  }

  constexpr bool operator != (Enum b)
  {
    return value == b.value;
  }

  constexpr bool operator < (Enum b)
  {
    return value < b.value;
  }

  constexpr bool operator > (Enum b)
  {
    return value > b.value;
  }

  constexpr bool operator >= (Enum b)
  {
    return value >= b.value;
  }

  constexpr bool operator <= (Enum b)
  {
    return value <= b.value;
  }

  Int value;
};

//! Switch printing enums as strings or integers. The
//! integer mode is default
std::ios_base& enumalpha(std::ios_base& ios);

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class T, class Int
>
std::basic_ostream<CharT, Traits>&
operator << 
  ( std::basic_ostream<CharT, Traits>& out,
    const Enum<T, Int>& e )
{
  return (out.iword(EnumBase::xalloc)) 
    ? out << e.name() 
    : out << e.value;
}

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class T, class Int
>
std::basic_istream<CharT, Traits>&
operator >> 
  ( std::basic_istream<CharT, Traits>& in, 
    Enum<T, Int>& e )
{
  if (in.iword(EnumBase::xalloc)) {
    std::string name;
    in >> name;
    e = Enum<T, Int>(name.c_str());
  }
  else in >> e.value;
  return in;
}

}

#endif


