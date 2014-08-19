// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * enum-like constexpr class.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
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


