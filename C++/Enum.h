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

#ifndef CONCURRO_ENUM_H_
#define CONCURRO_ENUM_H_

namespace curr {

constexpr bool string_equal
  (const char* a, const char* b)
{
  return a[0] == b[0]
    && (*a == 0 || string_equal(a+1, b+1));
}

#if 0
template<class Int, class Val0, class... Vals>
class EnumMeta : public EnumMeta<Int, Vals...>
{
public:
  typedef EnumMeta<Int, Vals...> parent;

  constexpr EnumMeta(Int k, Val0 val0, Vals... vals)
    : parent(k+1, vals...),
      value_(k),
      name_(val0)
  {}

  constexpr Val0 name(Int k)
  {
    return (k == value_) ? name_ : parent::name(k);
  }
  
  constexpr Int value(Val0 n)
  {
    return string_equal(n, name_) 
      ? value_ : parent::value(n);
  }

  const Int value_;
  const Val0 name_;
};

template<class Int, class Val0>
class EnumMeta<Int, Val0>
{
public:
  constexpr EnumMeta(Int k, Val0 val) 
    : value_(k), 
      name_(val)
  {}

  constexpr Val0 name(Int k)
  {
    return (k == value_) 
      ? name_ 
      : (throw std::domain_error("Bad enum value"));
  }
  
  constexpr Int value(Val0 n)
  {
    return string_equal(n, name_) 
      ? value_ 
      : (throw std::domain_error("Bad enum constant"));
  }

  const Int value_;
  const Val0 name_;
};
#else

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

#endif

#if 1

template<class V0, class... V/*, class Int = int*/>
constexpr EnumMeta<int, V0, 1 + sizeof...(V)> 
//
BuildEnum(V0 v0, V... v)
{
  return EnumMeta<int, V0, 1 + sizeof...(V)> { v0, v... };
}

template<class T, class Int = int>
class Enum
{
public:
#if 0
  template<std::size_t N, class... Vals>
  constexpr Enum(const char (&name)[N], Vals... vals)
    : value(T::meta().value(name))
  {
  }
#else
  template<class String>
  constexpr Enum(String name)
    : value(T::meta.value(name))
  {
  }
#endif

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

#else
template<class... V, class Int = int>
static constexpr EnumMeta<Int, V...> BuildEnum(V... vs)
{
  return EnumMeta<Int, V...>(0, vs...);
}

template<class T, class Int = int>
class Enum
{
public:
  template<std::size_t N, class... Vals>
  constexpr Enum(const char (&name)[N], Vals... vals)
   : value(T::meta().value(name))
  {
/*    static_assert
      (T::meta.value(name) != -1,
       "Enum: bad value");
*/
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
#endif


}

#endif


