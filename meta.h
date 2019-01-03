// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Metaprogramming primitives.
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


#ifndef TYPES_META_H_
#define TYPES_META_H_

#include <type_traits>
#include <tuple>
#include <utility>

namespace types { 

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

template<class Fun, class Tuple, int... S, class... Pars>
auto call_(Fun fun, seq<S...>, Tuple&& tup, Pars&&... pars)
  -> decltype(fun(
         std::forward<Pars>(pars)..., // non-tuple parameters
         std::get<S>(std::forward<Tuple>(tup))...
     ))
{
  return fun(
    std::forward<Pars>(pars)..., // non-tuple parameters
    std::get<S>(std::forward<Tuple>(tup))...
  );
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

template<class Fun, class Tuple, class... Pars2>
auto call(Fun fun, Tuple&& pars, Pars2&&... pars2) 
  -> decltype(
    call_(
      fun,
      typename gens<std::tuple_size<Tuple>::value>::type(),
      std::forward<Tuple>(pars),
      std::forward<Pars2>(pars2)...
    )
  )
{
  return call_<Fun>(
    fun,
    typename gens<std::tuple_size<Tuple>::value>::type(),
    std::forward<Tuple>(pars),
    std::forward<Pars2>(pars2)...
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

//! Iteration over std::tuple
template<
  template<class> class UnaryFunction, 
  std::size_t I = 0, 
  class... T,
  class... P
>
bool for_each(const std::tuple<T...>& t, P&&... pars)
{
  return UnaryFunction
    <typename std::tuple_element<I, decltype(t)>::type>
      (std::forward<P>(pars)...) // constructor parameters
      (std::get<I>(t))
    && for_each<UnaryFunction, I + 1, T..., P...>(t);
}

template<
  template<class> class UnaryFunction, 
  template<class> class... PT,
  std::size_t I = 0, 
  class... T,
  class... P
>
bool for_eachT(const std::tuple<T...>& t, P&&... pars)
{
  using TPar = typename std::tuple_element<I, decltype(t)>::type;

  return UnaryFunction<TPar>
      (PT<TPar>()..., std::forward<P>(pars)...) // constructor parameters
      (std::get<I>(t))
      && for_eachT<UnaryFunction, PT..., I + 1, T..., P...>(t);
}

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

template<class A, class B, class Enable = void>
struct is_member;

template<class A>
struct is_member<A, std::tuple<>> : std::false_type {};

template<class A, class... B>
struct is_member<A, std::tuple<A, B...>> : std::true_type {};

template<class A, class B0, class... B>
struct is_member<A, std::tuple<B0, B...>>
: std::integral_constant<bool, is_member<A, std::tuple<B...>>::value>
{
};

template<class A, class B, class Enable = void>
struct is_subset;

template<class... B>
struct is_subset< std::tuple<>, std::tuple<B...> > : std::true_type
{
};

template<class A0, class... A, class... B>
struct is_subset<
  std::tuple<A0, A...>,
  std::tuple<B...>
>
: std::integral_constant<
    bool,
    is_member<A0, std::tuple<B...>>::value
    && is_subset<std::tuple<A...>, std::tuple<B...>>::value
>
{
};

} // types

#endif
