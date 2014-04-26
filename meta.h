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
 * Metaprogramming primitives.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_TYPES_META_H_
#define CONCURRO_TYPES_META_H_

#include <type_traits>

namespace curr { 

//! Check whether `Check' is true (it is typically a
//! metaprogramming predicate with a value member of a
//! type bool)
//!
//! template<class T>
//! auto fun(...) -> 
//! EnableFunIf(is_same<T, QString>, FunReturnType)&
template<class Check, class RetType>
using EnableFunIf = typename std::remove_reference <
  decltype(typename std::enable_if<Check::value>::type(),
  std::declval<RetType>()) 
> :: type;

namespace types {

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

}}

#endif
