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

//#include <atomic>
//#include <tuple>
#include <iterator>
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

#if 0
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
#endif

/*  [========================[   is_char   ]=========================]*/
                             
template<class CharT>
struct is_character : std::false_type {};

template<> struct is_character<char> : std::true_type {};
template<> struct is_character<signed char> : std::true_type {};
template<> struct is_character<unsigned char> : std::true_type {};
template<> struct is_character<wchar_t> : std::true_type {};
template<> struct is_character<char16_t> : std::true_type {};
template<> struct is_character<char32_t> : std::true_type {};
template<> struct is_character<const char> : std::true_type {};
template<> struct is_character<const signed char> : std::true_type {};
template<> struct is_character<const unsigned char> : std::true_type {};
template<> struct is_character<const wchar_t> : std::true_type {};
template<> struct is_character<const char16_t> : std::true_type {};
template<> struct is_character<const char32_t> : std::true_type {};


/*  [==================[ is_stl_sequence ]===================]*/
                             
template<class T, class Enabled = void>
struct is_stl_sequence : std::false_type {};

template<class T>
struct is_stl_sequence<
  T,
  typename std::enable_if<
    std::is_same<
      decltype(std::distance(
        std::declval<T>().begin(),
        std::declval<T>().end()
      )),
      typename T::difference_type
   >::value
 >::type
> : std::true_type {};

template<class T, class Enabled = void>
struct is_safe_string : std::false_type {};

template<class T>
struct is_safe_string<
  T,
  typename std::enable_if<
    is_stl_sequence<T>::value
    && is_character<typename T::value_type>::value
  >::type
> : std::true_type {};

} // types

namespace strings
{
/*  [====================[   strings::traits   ]=====================]  */

// Forwards

template<class String, class Enable = void>
struct can_be_nullptr : std::false_type
{
};

template<class String>
struct can_be_nullptr<
    String,
    typename std::enable_if<
        std::is_same<
            decltype(std::declval<String>().is_nullptr()),
            bool
        >::value
    >::type
>
    : std::true_type
{
};
    
//! The type for unversal string access (all types of possible strings)
template<
    class T, 
    std::size_t SafetyLimit = ::strings::default_safety_limit, 
    class Enabled = void
>
struct traits;

//! STL-like string specialization
template<class String, std::size_t SafetyLimit>
struct traits<
    String,
    SafetyLimit,
    typename std::enable_if<
           types::is_string<String>::value
        && !std::is_pointer<String>::value
        && !std::is_array<String>::value
    >::type
>
{
    constexpr static std::size_t safety_limit = SafetyLimit;

    // NB no allocator_type
    using traits_type = typename String::traits_type;
    using value_type = typename String::value_type;
    using size_type = typename String::size_type;
    using difference_type = typename String::difference_type;
    using reference = typename String::reference;
    using const_reference = typename String::const_reference;
    using pointer = typename String::pointer;
    using const_pointer = typename String::const_pointer;
    using iterator = typename String::iterator;
    using const_iterator = typename String::const_iterator;
    using reverse_iterator = typename String::reverse_iterator;
    using const_reverse_iterator = typename String::const_reverse_iterator;

    constexpr static bool has_o1_size = arrays::has_o1_size<String>::value;

    static_assert(
        SafetyLimit <= 
            (std::size_t) std::numeric_limits<difference_type>::max(),
        "strings::traits: SafetyLimit value is too hight"
    );

    static size_type size(const String& s) 
    {
        const size_type res = s.size();
        assert(res <= safety_limit);
        return res;
    }

    //! Represents as a string with STL-like interface
    constexpr static String& generic(String& s)
    {
        return s;
    }

    constexpr static const String& generic(const String& s)
    {
        return s;
    }

    //! Some strings implementation holds pointer.
    //! If the pointer is nullptr the string considered empty()
    //! and is equal to any non-nullptr empty string.
    //! But on copying we must preserve nullptr behaviour
    //! for legacy code.
    template<
        class String2,
        typename std::enable_if<
               std::is_same<String, String2>::value
            && can_be_nullptr<String2>::value,
            bool
        >::type = false
        // enable only for types which can hold nullptr
        // (they must provide is_nullptr())
    >
    static bool is_nullptr(const String2& s)
    {
        return s.is_nullptr();
    }

    template<
        class String2,
        typename std::enable_if<
               std::is_same<String, String2>::value
            && !can_be_nullptr<String2>::value,
            bool
        >::type = false
        // enable only for types which can NOT hold nullptr
    >
    constexpr static bool is_nullptr(const String2&)
    {
        return false;
    }
};

//! CharT[M] string specialization
template<class CharT, std::size_t M, std::size_t SL>
struct traits<
    CharT[M],
    SL,
    typename std::enable_if<types::is_character<CharT>::value>::type
>
{
    constexpr static std::size_t safety_limit = SL;
    static_assert(M-1 < SL, "safety limit");

    // NB no allocator_type
    using value_type = typename std::remove_const<CharT>::type;
    using traits_type = std::char_traits<value_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    size_type size(const value_type*) const
    {
        return std::min(M, (std::size_t) 0);
    }

    constexpr static bool is_nullptr(const value_type*)
    {
        return false;
    }

    //! Represents as a string with STL-like interface
    template<
        class C,
        typename 
            std::enable_if<!std::is_const<C>::value, bool>::type = false
    >
    static basic_char_array<M, C, traits_type>& generic(C (&s)[M])
    {
        return adapter(s);
    }

    template<
        class C,
        typename 
            std::enable_if<std::is_const<C>::value, bool>::type = false
    >
    static auto generic(C (&s)[M]) -> decltype(adapter(s))
    {
        return adapter(s);
    }
};

//! CharT* string specialization
template<class CharT, std::size_t SafetyLimit>
struct traits<
    CharT*,
    SafetyLimit,
    typename std::enable_if<types::is_character<CharT>::value>::type
>
{
    constexpr static std::size_t safety_limit = SafetyLimit;

    // NB no allocator_type
    using value_type = typename std::remove_const<CharT>::type;
    using traits_type = std::char_traits<value_type>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static_assert(
        SafetyLimit <= 
            (std::size_t) std::numeric_limits<difference_type>::max(),
        "strings::traits: SafetyLimit value is too hight"
    );

    size_type size(const value_type* s) const
    {
        const size_type res = safe_strlen(s, safety_limit);
        assert(res <= safety_limit);
        return res;
    }

    static bool is_nullptr(const CharT* ptr)
    {
        return ptr == nullptr;
    }

    //! Represents as a string with STL-like interface
    template<
        class C,
        typename 
            std::enable_if<!std::is_const<C>::value, bool>::type = false
    >
    static basic_char_ptr<C, SafetyLimit>& generic(C* s)
    {
        return ptr_adapter(s);
    }

    template<
        class C,
        typename 
            std::enable_if<std::is_const<C>::value, bool>::type = false
    >
    static basic_const_char_ptr<C, SafetyLimit>& generic(C* s)
    {
        return ptr_adapter(s);
    }
};

//! Strings are compatible when they use the same traits type.
template<class String1, class String2, class Enable = void>
struct is_compatible : std::false_type {};

template<class String1, class String2>
struct is_compatible<
    String1,
    String2,
    typename std::enable_if<
        std::is_same<
            typename traits<String1>::traits_type,
            typename traits<String2>::traits_type
        >::value
    >::type
> : std::true_type
{
};

} // namespace strings

#endif



