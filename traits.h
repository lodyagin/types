// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Some type traits.
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


#ifndef TYPES_TRAITS_H
#define TYPES_TRAITS_H

#include <type_traits>
#include <tuple>

namespace types 
{

namespace pack
{

template<class... Args>
struct type {};

// Adds C into the Pack
template<class C, class Pack>
struct add;

template<class C, class... Args>
struct add<C, type<Args...>>
{
    typedef pack::type<C, Args...> type;
};

} // pack

namespace traits
{

// Static asserts that all Ds are descendants of B
template<class B, class... Ds>
struct is_base_of_any;

template<class B, class D>
struct is_base_of_any<B, D>
{
    static constexpr bool value = std::is_base_of<B, D>::value;
};

template<class B, class D, class... Ds>
struct is_base_of_any<B, D, Ds...>
{
    static constexpr bool value = std::is_base_of<B, D>::value
        && is_base_of_any<B, Ds...>::value;
};

namespace helper_
{

// Static asserts that at least one of Ds is a descendant of B.
// is_base_of_some::descendant is a name of the first Ds
// which is a descendant of B or void.
template<class B, class Ds, class = void>
struct is_base_of_some : std::false_type 
{
    typedef void descendant; // no descendants of B in Ds
};

// D0 is a descendant of B case
// It uses SFINAE techniques from
// http://stackoverflow.com/a/13949007/1326885
template<class B, class D0, class... Ds>
struct is_base_of_some<
    B, pack::type<D0, Ds...>,
    typename std::enable_if<std::is_base_of<B, D0>::value>::type
> : std::true_type 
{
    typedef D0 descendant;
};

// D0 is not a descendant of B case
template<class B, class D0, class... Ds>
struct is_base_of_some<
    B, pack::type<D0, Ds...>,
    typename std::enable_if<!std::is_base_of<B, D0>::value>::type
> : is_base_of_some<B, pack::type<Ds...>> {};

// Selects only B descendants from Ds and represents it as 
// a `descendants' member tuple type.
template<class B, class Ds, class = void>
struct select_descendants
{
    typedef pack::type<> descendants; // no descendants of B in Ds
};

// D0 is a descendant of B case
template<class B, class D0, class... Ds>
struct select_descendants<
    B, pack::type<D0, Ds...>,
    typename std::enable_if<std::is_base_of<B, D0>::value>::type
>
{
    typedef typename pack::add<
        D0, 
        typename select_descendants<B, pack::type<Ds...>>::descendants
    >::type descendants;
};

// D0 is not a descendant of B case
template<class B, class D0, class... Ds>
struct select_descendants<
    B, pack::type<D0, Ds...>,
    typename std::enable_if<!std::is_base_of<B, D0>::value>::type
>
{
    typedef typename select_descendants<B, pack::type<Ds...>>::descendants
        descendants;
};

} // helper_

template<class B, class D0, class... Ds>
struct is_base_of_some 
    : helper_::is_base_of_some<B, pack::type<D0, Ds...>>
{};

template<class B, class D0, class... Ds>
struct select_descendants 
    : helper_::select_descendants<B, pack::type<D0, Ds...>>
{};

} // traits

} // types

#endif



