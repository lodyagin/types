/**
 * @file
 * enum-like constexpr class.
 * You can easily convert between int value of enum and string name.
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
#include <iterator>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "types/typeinfo.h"

#ifndef TYPES_ENUM_H
#define TYPES_ENUM_H

namespace types {

namespace enum_ {

//! get a name of enum value
template<class EnumVal, class String = std::string>
String get_name()
{
  auto type_name = type_of<EnumVal>::template name<String>();
  // select only type name, discard namespace
  auto pos = type_name.find_last_of(':');
  if (pos != decltype(type_name)::npos)
    return type_name.substr(pos + 1);
  else
    return type_name;
};

//! The enum meta information type
template<class Int, Int N, class... Vals>
class meta;

template<class Int, Int N>
class meta<Int, N>
{
protected:
  static constexpr Int n = N;

public:
  using int_type = Int;

  template<class String = std::string>
  static const String& name() 
  { 
    return (String) "";
  }

  static constexpr Int index()
  {
    return n;
  }

protected:
  template<class It>
  static void fill_names(It) 
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

  template<class String = std::string>
  static const String& name(const Val&) 
  {
    static String the_name = get_name<Val, String>();
    return the_name;
  }

  static constexpr Int index(const Val&)
  {
    return n;
  }

protected:
  template<class It>
  static void fill_names(It it)
  {
    *it++ = std::cref(name(*(Val*)0));
    base::fill_names(it);
  }
};

static int xalloc()
{
  static int xalloc_ = std::ios_base::xalloc();
  return xalloc_;
}

//! Contains the names array
template<class Int, class... Vals>
class base
{
  using vector = std::vector<
    std::reference_wrapper<const std::string>
  >;
  using map = std::unordered_map<
    std::reference_wrapper<const std::string>,
    typename vector::size_type,
    std::hash<std::string>,
    std::equal_to<std::string>
  >;
  using dictionary = std::pair<vector, map>;

public:
  using int_type = Int;

  static const std::string& name(int_type idx)
  {
    const auto& names = dict().first;
    if (
      __builtin_expect(
        !(idx >= 0 && (decltype(names.size())) idx < names.size()),
        0
      ))
    {
      throw std::domain_error("the enum value is out of range");
    }
    return names[idx];
  }

  template<int_type NotFound, class String>
  static int_type lookup(const String& s)
  {
    const auto& indexes = dict().second;
    const auto it = indexes.find(s);
    return (it != indexes.end()) ? it->second : NotFound;
  }

private:
  static dictionary& dict() 
  {
    static dictionary the_dict = build_dictionary();
    return the_dict;
  }

  static dictionary build_dictionary()
  {
    dictionary d;
    d.first.reserve(sizeof...(Vals));
    meta<Int, sizeof...(Vals), Vals...>
      ::fill_names(std::back_inserter(d.first));
    for (size_t i = 0; i < d.first.size(); i++)
    {
        d.second[d.first[i]] = i;
    }
    return d;
  }
};

} // enum_


/* =======================[   enumerate   ]======================== */

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

  enumerate() noexcept {}

  explicit enumerate(Int i) : idx(i) {}

  template<
    class EnumVal,
    decltype(index(*(EnumVal*)0)) = 0
  >
  enumerate(const EnumVal& val) : idx(index(val)) {}

  const std::string& name() const
  {
    return base::name(idx);
  }

  int_type index() const
  {
    return idx;
  }

  template<int_type NotFound, class String>
  static enumerate parse(const String& s)
  {
    return enumerate(base::template lookup<NotFound>(s));
  }

  static constexpr std::size_t size()
  {
    return sizeof...(Vals);
  }

  bool operator==(const enumerate& b) const
  {
    return idx == b.idx;
  }

  template<
    class E2,
    decltype(index(E2())) = 0
  >
  bool operator==(const E2& b) const
  {
    return index(b) == idx;
  }

  template<class E1, class I2, class... Vs2>
  friend bool operator==(const E1& a, const enumerate<I2, Vs2...>& b);

#if 0
  template<
    class String,
    decltype(std::declval<String>().substr(0, 1)) = 0
  >
  bool operator==(const String& b) const
  {
    return b == name();
  }

  bool operator==(const char* b) const
  {
    return name() == b;
  }

    // you need const char*, Int and uintmax_t for eliminate errors
  bool operator==(Int) const;
  bool operator==(uintmax_t) const;
#endif

protected:
  Int idx;
};

template<class E1, class I2, class... Vs2>
bool operator==(const E1& a, const enumerate<I2, Vs2...>& b)
{
  return b.operator==(a);
}

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
  if (out.iword(enum_::xalloc()))
  {
    out << (intmax_t) v.index(); // prevent printing 
                                 // int8_t as char
  }
  else
  {
    try
    {
      out << v.name();
    }
    catch (...)
    {
      out << '*';
    }
  }
  return out;
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


