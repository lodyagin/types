// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * enum-like constexpr class.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#include <ios>
#include <cstddef>
#include <stdexcept>

#ifndef TYPES_ENUM_H
#define TYPES_ENUM_H

namespace types {

constexpr bool string_equal
  (const char* a, const char* b)
{
  return a[0] == b[0]
    && (*a == 0 || string_equal(a+1, b+1));
}

template<class Int, class String, std::size_t N>
class enum_meta
{
public:
  static constexpr std::size_t n = N;

  constexpr String name(Int k) const
  {
    return (k >= 0 && k < N) 
      ? names[k] 
      : (throw std::domain_error("Bad enum value"));
  }

  constexpr Int value(String name) const
  {
    return search_value(names, 0, name);
  }

  String names[N];

protected:
  constexpr Int search_value 
    (String const* arr, std::size_t k, String name) const
  {
    return (k < N) ?
      (string_equal(*arr, name) ? 
        k : search_value(arr + 1, k + 1, name))
      : (throw std::domain_error("Bad enum constant"));
  }
};

template<class V0, class... V/*, class Int = int*/>
constexpr enum_meta<int, V0, 1 + sizeof...(V)> 
//
build_enum(V0 v0, V... v)
{
  return enum_meta<int, V0, 1 + sizeof...(V)> { { v0, v... } };
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
  std::size_t N,
  class Int = int,
  class String = const char*
>
class enum_t : public EnumBase
{
public:
  using meta_type = enum_meta<Int, String, N>;

  constexpr enum_t() : value(0)
  {
  }

  constexpr enum_t(String name) : value(T::meta().value(name))
  {
  }

  constexpr String name() const
  {
    return T::meta().name(value);
  }

  constexpr bool operator == (enum_t b) const
  {
    return value == b.value;
  }

  constexpr bool operator != (enum_t b) const
  {
    return value == b.value;
  }

  constexpr bool operator < (enum_t b) const
  {
    return value < b.value;
  }

  constexpr bool operator > (enum_t b) const
  {
    return value > b.value;
  }

  constexpr bool operator >= (enum_t b) const
  {
    return value >= b.value;
  }

  constexpr bool operator <= (enum_t b) const
  {
    return value <= b.value;
  }

  enum_t& operator ++()
  {
    assert(value >= 0);
    assert(value < meta_type::n);
    ++value;
    return *this;
  }

  Int value;
};

//! Switch printing enums as strings or integers. The
//! integer mode is default
std::ios_base& enumalpha(std::ios_base& ios);

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class T, 
  std::size_t N,
  class Int
>
std::basic_ostream<CharT, Traits>&
operator << 
  ( std::basic_ostream<CharT, Traits>& out,
    const enum_t<T, N, Int>& e )
{
  return (out.iword(EnumBase::xalloc)) 
    ? out << e.name() 
    : out << e.value;
}

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class T, 
  std::size_t N,
  class Int
>
std::basic_istream<CharT, Traits>&
operator >> 
  ( std::basic_istream<CharT, Traits>& in, 
    enum_t<T, N, Int>& e )
{
  if (in.iword(EnumBase::xalloc)) {
    std::string name;
    in >> name;
    e = enum_t<T, N, Int>(name.c_str());
  }
  else in >> e.value;
  return in;
}

} // types

#endif


