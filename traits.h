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

#include <atomic>
#include <tuple>
#include <type_traits>
#include <utility>

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

// is_xxx valid also for signed xxx and unsigned xxx

#define TYPES_IS_XXX_DECL(xxx) \
template<class T> \
struct is_##xxx : std::false_type \
{ \
};

#define TYPES_IS_XXX_SPEC(xxx, yyy) \
template<> \
struct is_##xxx<yyy> : std::true_type \
{ \
};

TYPES_IS_XXX_DECL(char);
TYPES_IS_XXX_SPEC(char, char);
TYPES_IS_XXX_SPEC(char, signed char);
TYPES_IS_XXX_SPEC(char, unsigned char);

TYPES_IS_XXX_DECL(short);
TYPES_IS_XXX_SPEC(short, short);
TYPES_IS_XXX_SPEC(short, unsigned short);

TYPES_IS_XXX_DECL(int);
TYPES_IS_XXX_SPEC(int, int);
TYPES_IS_XXX_SPEC(int, unsigned int);

TYPES_IS_XXX_DECL(long);
TYPES_IS_XXX_SPEC(long, long);
TYPES_IS_XXX_SPEC(long, unsigned long);

template<class T>
struct is_long_long : std::false_type
{ 
};

template<>
struct is_long_long<long long> : std::true_type
{ 
};

template<>
struct is_long_long<unsigned long long> : std::true_type
{ 
};

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

// Atomicity checks 

template<class T>
struct is_atomic : std::false_type 
{
};

template<class T>
struct is_atomic<std::atomic<T>> : std::true_type
{
};

// Wait-free check

template<class Container, class = void>
struct is_wait_free : std::false_type
{
};

// std::array is wait-free
template<class T, size_t MaxSize>
struct is_wait_free<std::array<T, MaxSize>> : std::true_type
{
}; 

// Container marked as wait-free is wait-free
template<class Container>
struct is_wait_free<
    Container,
    typename std::enable_if<Container::is_wait_free()>::type
> : std::true_type
{
};

// std::atomic is wait-free if it is lock-free (depending on T)
template<>
struct is_wait_free<std::atomic<bool>> 
: 
#if ATOMIC_BOOL_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<class T>
struct is_wait_free<
    std::atomic<T>,
    typename std::enable_if<is_char<T>::value>::type
> 
: 
#if ATOMIC_CHAR_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<>
struct is_wait_free<std::atomic<char16_t>> 
: 
#if ATOMIC_CHAR16_T_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<>
struct is_wait_free<std::atomic<char32_t>> 
: 
#if ATOMIC_CHAR32_T_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<>
struct is_wait_free<std::atomic<wchar_t>> 
: 
#if ATOMIC_WCHAR_T_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<class T>
struct is_wait_free<
    std::atomic<T>,
    typename std::enable_if<is_short<T>::value>::type
> 
: 
#if ATOMIC_SHORT_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<class T>
struct is_wait_free<
    std::atomic<T>,
    typename std::enable_if<is_int<T>::value>::type
> 
: 
#if ATOMIC_INT_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<class T>
struct is_wait_free<
    std::atomic<T>,
    typename std::enable_if<is_long<T>::value>::type
> 
: 
#if ATOMIC_LONG_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<class T>
struct is_wait_free<
    std::atomic<T>,
    typename std::enable_if<is_long_long<T>::value>::type
> 
: 
#if ATOMIC_LLONG_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

template<class T>
struct is_wait_free<std::atomic<T*>> 
: 
#if ATOMIC_POINTER_LOCK_FREE == 2
    std::true_type
#else
    std::false_type
#endif
{
};

// nonatomic subtype if provided
template<class T, class = void>
struct nonatomic
{
    typedef T type;
};

template<class T>
struct nonatomic<T, typename T::nonatomic>
{
    typedef typename T::nonatomic type;
};

template<class T>
struct nonatomic<std::atomic<T>>
{
    typedef T type;
};

#if 1
template<class U1, class U2>
struct nonatomic<std::pair<U1, U2>>
{
    typedef std::pair<
        typename nonatomic<U1>::type,
        //typename nonatomic<U2>::type
        typename U2::nonatomic
    > type;
};
#endif

} // types

#endif



