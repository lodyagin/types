// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * enum-like constexpr class.
 *
 * This file (originally) was a part of public
 * https://github.com/lodyagin/types repository.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (c) 2014, Sergei Lodyagin
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the
 * above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ios>
#include <cassert>
#include <functional>
#include <vector>
#include <iterator>
#include "types/typeinfo.h"

#ifndef TYPES_ENUM_H
#define TYPES_ENUM_H

namespace types {

namespace enum_ {

//! get a name of enum value
template<class EnumVal>
std::string get_name()
{
  auto type_name = type<EnumVal>::name();
  // select only type name, discard namespace
  auto pos = type_name.find_last_of(':');
  if (pos != decltype(type_name)::npos)
    return type_name.substr(pos + 1);
  else
    return type_name;
};

template<class Int, Int N, class... Vals>
class meta;

template<class Int, Int N>
class meta<Int, N>
{
protected:
  static constexpr Int n = N;

public:
  using int_type = Int;

  static const std::string& name() 
  { 
    static std::string empty_name;
    return empty_name;
  }

  static constexpr Int index()
  {
    return n;
  }

protected:
  template<class It>
  static void fill_names(It it) 
  {
  }
};

template<class Int, Int N, class Val, class... Vals>
class meta<Int, N, Val, Vals...> 
  : public meta<Int, N, Vals...>
{
  template<class I, class... V>
  friend class base;

  using base = meta<Int, N, Vals...>;

protected:
  static constexpr Int n = base::n - 1;

public:
  using base::name;
  using base::index;

  static const std::string& name(Val) 
  {
    static std::string the_name = get_name<Val>();
    return the_name;
  }

  static constexpr Int index(Val)
  {
    return n;
  }

protected:
  template<class It>
  static void fill_names(It it)
  {
    *it++ = std::cref(name(Val()));
    base::fill_names(it);
  }
};

static int xalloc()
{
  static int xalloc_ = std::ios_base::xalloc();
  return xalloc_;
}

template<class Int, class... Vals>
class base
{
  using vector = std::vector<
    std::reference_wrapper<const std::string>
  >;

public:
  using int_type = Int;

  static const std::string& name(int_type idx)
  {
    static vector names = init_names();
    assert(idx >= 0 && idx < names.size());
    return names[idx];
  }

private:
  static vector init_names()
  {
    vector res;
    res.reserve(sizeof...(Vals));
    meta<Int, sizeof...(Vals), Vals...>
      ::fill_names(std::back_inserter(res));
    return res;
  }
};

} // enum_

template<class Int, class... Vals>
class enumerate 
  : public enum_::meta<Int, sizeof...(Vals), Vals...>,
    public enum_::base<Int, Vals...>
{
  using meta = enum_::meta<Int,sizeof...(Vals),Vals...>;
  using base = enum_::base<Int, Vals...>;

public:
  using typename base::int_type;

  using meta::name;
  using meta::index;

  enumerate() {}

  template<class EnumVal>
  enumerate(EnumVal val) : idx(index(val)) {}

  const std::string& name() const
  {
    return base::name(idx);
  }

  int_type index() const
  {
    return idx;
  }

  static constexpr std::size_t size()
  {
    return sizeof...(Vals);
  }

protected:
  Int idx;
};

// i.e. enum_type_index<red>() or enum_type_index(red())
template<class EnumVal>
struct enum_type_index
{
  enum_type_index() {}
  enum_type_index(EnumVal val) {}

  operator std::type_index() const
  {
    return std::type_index(typeid(EnumVal));
  }
};

// i.e. colour = red(); enum_type_index(colour);
template<class Int, class... Vals>
struct enum_type_index<enumerate<Int, Vals...>>
{
  // TODO
};

//! Switch printing enums as strings or integers. The
//! string mode is default
inline std::ios_base& enumalpha(std::ios_base& ios)
{
  ios.iword(enum_::xalloc()) = 0;
  return ios;
}

inline std::ios_base& noenumalpha(std::ios_base& ios)
{
  ios.iword(enum_::xalloc()) = 1;
  return ios;
}

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Int,
  class... EnumVals
>
std::basic_ostream<CharT, Traits>&
operator << ( 
  std::basic_ostream<CharT, Traits>& out,
  enumerate<Int, EnumVals...> v
)
{
  return (out.iword(enum_::xalloc())) 
    ? out << (intmax_t) v.index() // prevent printing 
                                  // int8_t as char
    : out << v.name();
}

#if 0
template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Int,
  class... EnumVals
>
std::basic_istream<CharT, Traits>&
operator >> ( 
  std::basic_istream<CharT, Traits>& in, 
  enumerate<Int, EnumVals...>& e 
)
{
  if (in.iword(EnumBase::xalloc)) {
    std::string name;
    in >> name;
    e = enum_t<T, N, Int>(name.c_str());
  }
  else in >> e.value;
  return in;
}
#endif

} // types

#endif


