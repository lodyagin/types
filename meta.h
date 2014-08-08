/* -*-coding: mule-utf-8-unix; fill-column: 58; -*- *******

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
 * Metaprogramming primitives.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_TYPES_META_H_
#define CONCURRO_TYPES_META_H_

#include <type_traits>
#include <tuple>
#include <utility>

namespace curr { 

//! Check whether `Check' is true (it is typically a
//! metaprogramming predicate with a value member of a
//! type bool)
//!
//! template<class T>
//! auto fun(...) -> 
//! enable_fun_if(is_same<T, QString>, FunReturnType)&
template<class Check, class RetType>
using enable_fun_if = typename std::remove_reference <
  decltype(typename std::enable_if<Check::value>::type(),
  std::declval<RetType>()) 
> :: type;

//! Check whether `Check' is false (it is typically a
//! metaprogramming predicate with a value member of a
//! type bool)
//!
//! template<class T>
//! auto fun(...) -> 
//! enable_fun_if_not(is_same<T, QString>, FunReturnType)&
template<class Check, class RetType>
using enable_fun_if_not = typename std::remove_reference <
  decltype(typename std::enable_if<!Check::value>::type(),
  std::declval<RetType>()) 
> :: type;

namespace tuple {

// tuple -> Args...
// Thanks to Johannes Schaub
// http://stackoverflow.com/a/7858971

template<int...>
struct seq { };

template<int N, int... S>
struct gens : gens<N-1, N-1, S...> { };

template<int... S>
struct gens<0, S...> {
  typedef seq<S...> type;
};

template<class Fun, class Tuple, int... S>
auto call_(seq<S...>, const Tuple& tup)
  -> decltype(Fun(std::get<S>(tup)...))
{
  return Fun(std::get<S>(tup)...);
}

template<class Fun, class Tuple>
auto call(const Tuple& pars) 
  -> decltype(
    call_<Fun>(
      typename gens<std::tuple_size<Tuple>::value>::type(),
      pars
    )
  )
{
  return call_<Fun>(
    typename gens<std::tuple_size<Tuple>::value>::type(),
    pars
  );
}

template<class Type, class Tuple, int... S>
auto aggregate_construct_(seq<S...>, const Tuple& tup)
  -> decltype(Type{std::get<S>(tup)...})
{
  return Type{std::get<S>(tup)...};
}

template<class Type, class Tuple>
auto aggregate_construct(const Tuple& pars) 
  -> decltype(
    aggregate_construct_<Type>(
      typename gens<std::tuple_size<Tuple>::value>::type(),
      pars
    )
  )
{
  return aggregate_construct_<Type>(
    typename gens<std::tuple_size<Tuple>::value>::type(),
    pars
  );
}

} // tuple
} // curr

namespace types {

//! Returns a most derived type
template<class T1, class T2, class Enabled = void>
struct most_derived;

template<class T>
struct most_derived<T, T>
{
  using type = T;
};

template<class T1, class T2>
struct most_derived<
  T1, 
  T2, 
  typename std::enable_if<
    std::is_base_of<T1, T2>::value
    && !std::is_same<T1, T2>::value
  >::type
>
{
  using type = T2;
};

template<class T1, class T2>
struct most_derived<
  T1, 
  T2, 
  typename std::enable_if<
    std::is_base_of<T2, T1>::value
    && !std::is_same<T1, T2>::value
  >::type
>
{
  using type = T1;
};

#if 0
namespace {

template<
  template<class...> class Parent,
  template<class...> class... Ancestors,
  class... Ts
>
class aggregate_ : public aggregate<Ancestors..., Ts...>
{
public:
  using aggregate<Ancestors..., Ts...>::aggregate;
};

}

struct end_of_templates {};

template<
  template<class...> class Parent,
  template<class...> class... Ancestors,
  class... Ts
>
class aggregate : public aggregate<Ancestors..., Ts...>
{
public:
  using aggregate<Ancestors..., Ts...>::aggregate;
};

template<template<class...> class Parent, class... Ts>
class aggregate<Parent, end_of_templates, Ts...>
#endif

//! Iteration over std::touple
template<
  //template<class> class UnaryFunction, 
  std::size_t I = 0, 
  class... T
>
//typename std::enable_if<I == sizeof...(T)>::type
void for_each(const std::tuple<T...>& t)
{
}

#if 0
template<
  template<class> class UnaryFunction, 
  std::size_t I = 0, 
  class... T
>
typename std::enable_if<I < sizeof...(T)>::type
for_each(std::tuple<T...>&& t)
{
  UnaryFunction
    <typename std::tuple_element<I, decltype(t)>::type>
      (std::get<I>(std::move(t)));
  for_each<UnaryFunction, I + 1, T...>(std::move(t));
}
#endif

//! This is type expression to check whether `base' is a
//! base of `derived'
#define CURR_ENABLE_BASE_TYPE(base, derived) \
  typename std::enable_if \
    <std::is_base_of<base, derived>::value>::type

//! The quirk to pass a template parameters as an macro
//! argument 
#define CURR_TEMPLATE_AND_PARS(templ, pars...) templ<pars>

#if 0
template<class C, class Enable = void>
struct ctr_args;


template<class C>
struct ctr_args<C, decltype(C())>
{
  static constexpr int n = 0;
};

template<class C, class A1>
struct ctr_args<C, decltype(C(std::declval<A1>()))>
{
  using tuple = std::tuple<A1>;
  static constexpr int n = 1;
};
#endif

} // types

#endif
