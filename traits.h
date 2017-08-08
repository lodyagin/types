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
#include <iterator>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
//#include "enum.h"
#ifdef BARE_CXX
#  include <bits/silent_assert.h>
#else
#  include <cassert>
#  define _silent_assert assert
#endif
#include <types/pair.h>

/* Logic extensions */

// tri-state logic 
namespace tristate
{

#if 0 // wait enumerate
struct True {};
struct False {};
struct Bottom {}; // http://stackoverflow.com/a/26430781/1326885

class Bool : public types::enumerate<char, True, False, Bottom>
{
    using base = types::enumerate<char, True, False, Bottom>;

public:
  Bool() noexcept : base(Bottom()) 
  {
  }

  template<class EnumVal>
  Bool(const EnumVal& val) : base(val) 
  {
  }

  Bool(bool distate) 
    : Bool((distate == true) ? Bool(tristate::True()) : Bool(tristate::False()))
  {
  }

  bool definitely_true() const
  {
    return *this == True();
  }

  bool definitely_false() const
  {
    return *this == False();
  }
};
#endif

} // namespace tristate

// forward declarations
namespace strings
{

constexpr std::size_t default_safety_limit = 16384;

#if 1

namespace zero_termination_
{
namespace static_size
{

template<std::size_t BuffSize, class CharT>
class force
{
public:
    //! Makes shure the string is zero-terminated.  Declared as const
    //! because it is allowed to call it in const context.
    void do_zero_termination() const noexcept
    {
        if (BuffSize > 0)
        {
            CharT* s = const_cast<CharT*>
                (reinterpret_cast<const CharT*>(this));

            s[BuffSize - 1] = CharT();
        }
    }
};

template<std::size_t BuffSize, class CharT>
class check
{
public:
    //! Makes shure the string is zero-terminated.
    //! @throw strings::not_terminated 
    void do_zero_termination() const
    {
        if (BuffSize > 0)
        {
            const CharT* s = reinterpret_cast<const CharT*>(this);
            if (__builtin_expect(s[BuffSize - 1] != CharT(), 0))
            {
                _silent_assert(false);
            }
        }
    }
};

} // namespace static_size

namespace dynamic_size
{

// we don't provide `force' for this case
// like in static_size ns just because it is risky
// (if size is set wrong we have no intention
// to bomb random memory part with zeroes).

template<class Base>
class check : public Base
{
public:
    //! Makes sure the string is zero-terminated.
    //! @throw strings::not_terminated 
    void do_zero_termination() const
    {
        const auto sz = this->size();
        assert(sz >= 0);
        if (sz > 0)
        {
            if (
                __builtin_expect(
                    this->ptr()[sz] != typename Base::value_type()
                    ,0
                )
               )
            {
                _silent_assert(false);
            }
        }
    }
};

} // namespace dynamic_size
} // namespace zero_termination_

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits = std::char_traits<CharT>,
    class Base = zero_termination_::static_size::check<MaxSize, CharT>,
    bool O1Size = false
>
class basic_const_char_array;

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits  = std::char_traits<CharT>,
    class BaseBase = zero_termination_::static_size::force<MaxSize, CharT>
>
class basic_char_array;

#if 1
template<
    class CharT,
    class Traits = std::char_traits<typename std::remove_const<CharT>::type>,
    std::size_t MaxSize,
    typename std::enable_if<std::is_const<CharT>::value, bool>::type
        = false
    >
const basic_const_char_array<
    MaxSize, 
    typename std::remove_const<CharT>::type, 
    Traits, 
    zero_termination_::static_size::check<
        MaxSize, 
        typename std::remove_const<CharT>::type
    >,
    false
>& 
//
adapter(CharT (&str)[MaxSize]);

template<
    class CharT,
    class Traits = std::char_traits<CharT>,
    std::size_t MaxSize,
    typename std::enable_if<!std::is_const<CharT>::value, bool>::type
        = false
    >
basic_char_array<
    MaxSize, 
    CharT, 
    Traits, 
    zero_termination_::static_size::force<MaxSize, CharT>
>& 
//
adapter(CharT (&str)[MaxSize]);
#endif
#endif

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  std::size_t MaxLen = std::numeric_limits<std::size_t>::max()
> 
class basic_constexpr_string;

template <
  class CharT,
  std::int16_t N,
  class Traits = std::char_traits<CharT>
> 
class basic_auto_string;

} // namespace strings

namespace types
{

// the same as std::min but constexpr
template<class T>
constexpr const T& min(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<class T>
constexpr const T& max(const T& a, const T& b)
{
    return (b > a) ? b : a;
}

} // namespace types

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

} // namespace pack

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

} // namespace helper_

template<class B, class D0, class... Ds>
struct is_base_of_some 
    : helper_::is_base_of_some<B, pack::type<D0, Ds...>>
{};

template<class B, class D0, class... Ds>
struct select_descendants 
    : helper_::select_descendants<B, pack::type<D0, Ds...>>
{};

#if 0
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
template<class T, std::size_t MaxSize>
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
#endif

#if 0
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

template<class U1, class U2>
struct nonatomic<std::pair<U1, U2>>
{
    typedef std::pair<
        typename nonatomic<U1>::type,
#if 1
        typename nonatomic<U2>::type
#else
        typename U2::nonatomic
#endif
    > type;
};

template<class U1, class U2>
struct nonatomic<types::pair<U1, U2>>
{
    typedef types::pair<
        typename nonatomic<U1>::type,
#if 0
        typename nonatomic<U2>::type
#else
        typename U2::nonatomic
#endif
    > type;
};
#endif

/*  [====================[   is_sequence   ]=========================]  */

template<class T, class Enable = void>
struct is_sequence : std::false_type
{
};


/*  [====================[   is_iterator   ]=========================]  */

//! T is iterator of ValueType
template<class T, class ValueType, class Enable = void>
struct is_iterator : std::false_type
{
};

template<class T, class ValueType>
struct is_iterator<
    T,
    ValueType,
    typename std::enable_if<
        std::is_same<
            typename std::iterator_traits<T>::value_type,
            ValueType
        >::value
    >::type
> : std::true_type
{
};

/*  [========================[   is_char   ]=========================]  */
                             
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

/*  [======================[   is_more_const   ]=====================]  */

//! A is more const than B
template<class A, class B, class Enable = void>
struct is_more_const : std::false_type {};

template<class A, class B>
struct is_more_const<
    A,
    B,
    typename std::enable_if<
        implies(std::is_const<B>::value, std::is_const<A>::value)
    >::type
> : std::true_type
{
};

/*  [========================[   char_cast   ]=======================]  */
                             
//! Casts to a different char type. It covers safe cases only.
template<
    class To, 
    class From,
    typename std::enable_if<
          std::is_pointer<To>::value
       && std::is_pointer<From>::value
       && is_character<typename std::remove_pointer<To>::type>::value 
       && is_character<typename std::remove_pointer<From>::type>::value 
       && sizeof(typename std::remove_pointer<To>::type) 
            == sizeof(typename std::remove_pointer<From>::type)
       && is_more_const<
            typename std::remove_pointer<To>::type, 
            typename std::remove_pointer<From>::type
          >::value,
       bool
    >::type = false
>
To char_cast(From p)
{
    return reinterpret_cast<To>(p);
}

/*  [=======================[   is_string   ]========================]  */

template<class T, class Enabled = void>
struct is_string : std::false_type {};

template<class CharT, std::size_t M>
struct is_string<
    CharT[M],
    typename std::enable_if<is_character<CharT>::value>::type
> 
  : std::true_type
{
};

#if 0
template<class CharT>
struct is_string<
    CharT*,
    typename std::enable_if<is_character<CharT>::value>::type
> 
  : std::true_type
{
};
#endif

#if 0 // wait till Allocator
template<class CharT, class Traits, class Allocator>
struct is_string<std::basic_string<CharT, Traits, Allocator>>
    : std::true_type
{
};
#endif

template<class CharT, class Traits, std::size_t MaxLen>
struct is_string<strings::basic_constexpr_string<CharT, Traits, MaxLen>>
    : std::true_type
{
};

template <
  class CharT,
  std::int16_t N,
  class Traits
>
struct is_string<strings::basic_auto_string<CharT, N, Traits>>
  : std::true_type
{
};

#if 1
template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base,
    bool  O1Size
>
struct is_string<
    strings::basic_const_char_array<MaxSize,CharT,Traits, Base, O1Size>
>
    : std::true_type
{
};

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base
>
struct is_string<
    strings::basic_char_array<MaxSize,CharT,Traits, Base>
>
    : std::true_type
{
};
#endif

/*  [======================[   is_sequence   ]=======================]  */

template<class String>
struct is_sequence<
    String,
    typename std::enable_if<is_string<String>::value>::type
>
  : std::true_type
{
};

/*  [===================[   sequence_size   ]========================]  */

template<
    class String,
    typename std::enable_if<is_string<String>::value, bool>::type = false
>
typename String::size_type sequence_size(const String& s)
{
    return s.size();
}

} // namespace types

namespace arrays
{

#if 1
// Forwards

namespace mem_layout
{

enum kind_of_size 
{ 
    number_of_elements 
};

} // namespace mem_layout
#endif

//! Whether size() has o(1) complexity
template<class String, class Enable = void>
struct has_o1_size : std::false_type {};

#if 0 // wait till vector, Allocator
template<class T, class Allocator>
struct has_o1_size<std::vector<T, Allocator>> : std::true_type 
{
  typedef std::vector<T, Allocator> array_type;
  typename array_type::size_type size_type;
  
  static size_type size(const array_type& a)
  {
    return a.size();
  }
};
#endif

template<class T, std::size_t N>
struct has_o1_size<std::array<T, N>, void> : std::true_type 
{
  typedef std::array<T, N> array_type;
  typedef typename array_type::size_type size_type;
  
  static constexpr size_type size(const array_type& a)
  {
    return N;
  }
};

#if 0 // wait till Allocator
template<class CharT, class Traits, class Allocator>
struct has_o1_size<std::basic_string<CharT, Traits, Allocator>> 
    : std::true_type
{
  typedef std::basic_string<CharT, Traits, Allocator> array_type;
  typedef typename array_type::size_type size_type;
  
  static size_type size(const array_type& a)
  {
    return a.size();
  }
};
#endif

#if 1
template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base,
    bool O1Size
>
struct has_o1_size<
    strings::basic_const_char_array<MaxSize, CharT, Traits, Base, O1Size>
> 
    : std::integral_constant<bool, O1Size>
{
};

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class BaseBase
>
struct has_o1_size<
    strings::basic_char_array<MaxSize, CharT, Traits, BaseBase>
> 
    : std::false_type // NB!
{
};
#endif

template <
  class CharT,
  class Traits,
  std::size_t MaxLen
> 
struct has_o1_size<
    strings::basic_constexpr_string<CharT, Traits, MaxLen>
>
    : std::true_type
{
  typedef strings::basic_constexpr_string<CharT, Traits, MaxLen> array_type;
  typedef typename array_type::size_type size_type;
  
  static size_type size(const array_type& a)
  {
    return a.size();
  }
};

template <
  class CharT,
  std::size_t M
> 
struct has_o1_size< CharT[M] > : std::true_type
{
  static constexpr std::size_t size(const CharT (&a)[M])
  {
    static_assert(M > 0, "M must be > 0");
    return M - 1;
  }
};

template <
  class CharT,
  std::int16_t N,
  class Traits
> 
struct has_o1_size<
    strings::basic_auto_string<CharT, N, Traits>
>
    : std::true_type
{
  typedef strings::basic_auto_string<CharT, N, Traits> array_type;
  typedef typename array_type::size_type size_type;
  
  static size_type size(const array_type& a)
  {
    return a.size();
  }
};

}

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
        _silent_assert(res <= safety_limit);
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

    size_type size(const_pointer) const
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

namespace arrays
{

/* =====================[   arrays::traits   ]========================== */

template<class Array, class Enabled = void>
struct traits;

// strings

#if 1

template<class String>
struct traits<
    String,
    typename std::enable_if<
        std::is_convertible<
            decltype(strings::traits<String>::generic(std::declval<String>()).size()), 
                // call generic to be sure the type is not incomplete
            std::size_t
        >::value
    >::type
> : strings::traits<String>
{
};

template<class CharT, std::size_t M>
struct traits< CharT[M] > : strings::traits<CharT[M]>
{
};

#else

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base,
    bool O1Size
>
struct traits<
    strings::basic_const_char_array<MaxSize, CharT,Traits,Base, O1Size>
>
{
    using array_type  = strings::basic_const_char_array<
        MaxSize, CharT, Traits, Base, O1Size
    >;
    using size_type = typename array_type::size_type;

    constexpr static bool has_o1_size = O1Size;
};

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base
>
struct traits<strings::basic_char_array<MaxSize, CharT,Traits,Base>>
{
    using array_type  = strings::basic_char_array<
        MaxSize, CharT,Traits,Base
    >;
    using size_type = typename array_type::size_type;

    constexpr static bool has_o1_size = true;
};


template<class CharT, std::size_t SafetyLimit>
struct traits<strings::basic_const_char_ptr<CharT, SafetyLimit>>
{
    using array_type  = strings::basic_const_char_ptr<CharT, SafetyLimit>;
    using size_type = typename array_type::size_type;

    constexpr static bool has_o1_size = false;
};

template<class CharT, std::size_t SafetyLimit>
struct traits<strings::basic_char_ptr<CharT, SafetyLimit>>
{
    using array_type  = strings::basic_const_char_ptr<CharT, SafetyLimit>;
    using size_type = typename array_type::size_type;

    constexpr static bool has_o1_size = false;
};

template<
    class CharT,
    class SizeT,
    std::ptrdiff_t SizeFieldOffset,
    mem_layout::kind_of_size KindOfSize,
    class Traits,
    class BaseCharT
>
struct traits<
    strings::basic_const_char_ptr_and_size<
        CharT,
        SizeT,
        SizeFieldOffset,
        KindOfSize,
        Traits,
        BaseCharT
    >
>
{
    using array_type  = 
    strings::basic_const_char_ptr_and_size<
        CharT,
        SizeT,
        SizeFieldOffset,
        KindOfSize,
        Traits,
        BaseCharT
    >;
    using size_type = typename array_type::size_type;

    constexpr static bool has_o1_size = true;
};

template<
    class CharT,
    class SizeT,
    std::ptrdiff_t SizeFieldOffset,
    mem_layout::kind_of_size KindOfSize,
    class Traits
>
struct traits<
    strings::basic_char_ptr_and_size<
        CharT,
        SizeT,
        SizeFieldOffset,
        KindOfSize,
        Traits
    >
>
{
    using array_type  = 
    strings::basic_char_ptr_and_size<
        CharT,
        SizeT,
        SizeFieldOffset,
        KindOfSize,
        Traits
    >;
    using size_type = typename array_type::size_type;

    constexpr static bool has_o1_size = true;
};
#endif

// arrays 

template<class T, std::size_t M>
struct traits<
    T[M],
    typename std::enable_if<!types::is_character<T>::value>::type
>
{
    using array_type  = T[M];
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
};

#if 0
template<class T, class Allocator>
struct traits<std::vector<T, Allocator>, void>
{
    using array_type  = std::vector<T, Allocator>;
    using size_type = typename array_type::size_type;
    using difference_type = typename array_type::difference_type;
};

template<class T, std::size_t N>
struct traits<std::array<T, N>, void>
{
    using array_type  = std::array<T, N>;
    using size_type = typename array_type::size_type;
    using difference_type = typename array_type::difference_type;
};

#endif

#if 0 // wait Bool
//! Whether size of s1 is equal to s2.
//! Returns tristate::Bottom if this could not be calculated in o(1).
template<class Sequence1, class Sequence2>
tristate::Bool equal_size(const Sequence1& s1, const Sequence2& s2)
{
    if (has_o1_size<Sequence1>::value && has_o1_size<Sequence2>::value)
    {
        return arrays::traits<Sequence1>::generic(s1).size() 
            == arrays::traits<Sequence2>::generic(s2).size();
    }
    else
    {
        return tristate::Bottom();
    }
}
#endif

} // namespace arrays

#if 0
/* Disable-enable depending on is_wait_free<> */

#define TYPES_WF_ENABLE_CORE(TYPE) \
    class This = TYPE, /* (do not specialize this!) */ \
    typename std::enable_if<types::is_wait_free<This>::value, bool> \
        ::type = false \

#define TYPES_WF_DISABLE_CORE(TYPE) \
    class This = TYPE, /* (do not specialize this!) */ \
    typename std::enable_if<!types::is_wait_free<This>::value, bool> \
        ::type = false \

#define TYPES_WF_ENABLE(TYPE) \
template< \
    TYPES_WF_ENABLE_CORE(TYPE) \
>

#define TYPES_WF_DISABLE(TYPE) \
template< \
    TYPES_WF_DISABLE_CORE(TYPE) \
>
#endif

namespace types
{

//! Like std::distance but with a limit
template<
    class InputIt,
    typename std::enable_if<
        std::is_convertible<
            typename std::iterator_traits<InputIt>::iterator_category,
            std::random_access_iterator_tag
        >::value,
        bool
    >::type = false
>
typename std::iterator_traits<InputIt>::difference_type
safe_distance(InputIt first, InputIt last, std::size_t lim)
{
    assert(last >= first);
    return std::min((std::size_t) (last - first), lim);
}

} // namespace types

namespace comparison
{

struct less {};
struct equal {};
struct greater {};
struct not_equal {};

#if 0 // waits enumerate
// Less-Equal-Greater
class LEG : public types::enumerate<char, less, equal, greater>
{
    using base = types::enumerate<char, less, equal, greater>;

public:
    LEG() = delete; // no default value

    constexpr LEG(less)      : base(less()) {}
    constexpr LEG(equal)     : base(equal())     {}
    constexpr LEG(greater)   : base(greater()) {}

    template<
        class Int,
        typename std::enable_if<std::is_signed<Int>::value, bool>::type = false
    >
    constexpr explicit LEG(Int i) 
        : LEG(
            (i == 0) ? 
                LEG(equal())
              : ( (i < 0) ? LEG(less()) : LEG(greater()))
          )
    {}

    //! Converts to {-1, 0, 1}, like result of std::memcmp
    constexpr explicit operator int() const noexcept
    {
        return index() - 1;
    }

    constexpr operator bool() const noexcept
    {
        return *this == equal();
    }
};

// Not equal - Equal
class ENE : public types::enumerate<char, not_equal, equal>
{
    using base = types::enumerate<char, not_equal, equal>;

public:
    ENE() = delete; // no default value

    constexpr ENE(less)      : base(not_equal()) {}
    constexpr ENE(equal)     : base(equal())     {}
    constexpr ENE(greater)   : base(not_equal()) {}
    constexpr ENE(not_equal) : base(not_equal()) {}

    constexpr ENE(bool eq) : base((eq) ? ENE(equal()) : ENE(not_equal())) {}

    constexpr operator bool() const noexcept
    {
        return *this == equal();
    }
};
#endif

//! A generic comparison functor
template<class Result>
struct compare;

#if 0 // waits enumerate
template<>
struct compare<ENE>
{
    template<class T>
    ENE operator()(const T& a, const T& b) const
    {
        return a == b;
    }
};

template<>
struct compare<LEG>
{
    template<class T>
    ENE operator()(const T& a, const T& b) const
    {
        if (a < b)
        {
            return less();
        }
        else if (a > b)
        {
            return greater();
        }
        else
        {
            return equal();
        }
    }
};
#endif

template<>
struct compare<bool>
{
    template<class T>
    bool operator()(const T& a, const T& b) const
    {
        return a == b;
    }
};

namespace character
{

template<class Result, class CharT, class Traits, class Enable = void>
struct compare;

#if 0 // waits enumerate
template<class Result, class CharT, class Traits>
struct compare<
    Result, 
    CharT, 
    Traits, 
    typename std::enable_if<
        std::is_convertible<Result, ENE>::value
    >::type
>
{
    Result operator()(CharT a, CharT b) const noexcept
    {
        return Traits::eq(a, b);
    }
};

template<class Result, class CharT, class Traits>
struct compare<
    Result, 
    CharT, 
    Traits, 
    typename std::enable_if<
        std::is_convertible<Result, LEG>::value
    >::type
>
{
    Result operator()(CharT a, CharT b) const noexcept
    {
        if (Traits::eq(a, b))
        {
            return equal();
        }
        else if (Traits::lt(a, b))
        {
            return less();
        }
        else return greater();
    }
};
#endif

} // namespace character

namespace sequence
{

#if 0 // waits enumerate
//! Generic sequence comparison. Can compare both as equal/not-equal
//! and greater/equal/lower.
//!  @tparam Result LEG or ENE
template<
    class Result, 
    class Sequence1, 
    class Sequence2, 
    template <class> class Compare = comparison::compare
>
Result compare(
    const Sequence1& a_, 
    const Sequence2& b_,
    //const Compare& comp = comparison::compare<Result>(),
    std::size_t safety_limit 
        = types::min(
            std::numeric_limits<
                typename ::arrays::traits<Sequence1>::difference_type
            >::max(),
            std::numeric_limits<
                typename ::arrays::traits<Sequence2>::difference_type
            >::max()
          )
)
{
    constexpr bool ene = std::is_convertible<Result, ENE>::value;
    constexpr bool leg = std::is_convertible<Result, LEG>::value; 
    static_assert(
        ene != leg /* XOR */, 
        "the unknown result type is requested for comparison::compare"
    );

    using Intermediate = typename std::conditional<ene, ENE, LEG>::type;

    // If no need to compare lexicographically then start with sizes
    // comparison
    if (ene && arrays::equal_size(a_, b_).definitely_false())
    {
        return (Intermediate) not_equal(); 
    }

    const auto& a = arrays::traits<Sequence1>::generic(a_);
    const auto& b = arrays::traits<Sequence2>::generic(b_);

    // Continue with pointer comparison (and the sizes again)
    if (   a.data() == b.data()
        && arrays::equal_size(a_, b_).definitely_true()
    )
    {
        return (Intermediate) equal();
    }

    using a_type = typename std::remove_const<
        typename std::remove_reference<decltype(a)>::type
    >::type;
    using b_type = typename std::remove_const<
        typename std::remove_reference<decltype(b)>::type
    >::type;

    if (__builtin_expect(arrays::traits<Sequence1>::is_bottom(a_), 0))
    {
        assert(!arrays::traits<Sequence2>::is_bottom(b_));
            // equal pointers case was already checked
        return (Intermediate) less(); 
           // bottom is always less than everything else
     }
    else
    {
        if (__builtin_expect(arrays::traits<Sequence2>::is_bottom(b_), 0))
        {
            return (Intermediate) greater();
        }
    }

    auto ait = a.begin();
    auto bit = b.begin();
    auto alast = a.end();
    auto blast = b.end();

    std::size_t cnt = safety_limit;
    const Compare<Intermediate> comp{};
    while (ait != alast && bit != blast && comp(*ait, *bit)) 
    {
        if (__builtin_expect(--cnt == 0, 0))
            break;
        ++ait; ++bit;
    }
    assert(cnt > 0); // breaking the safety limit is an error
    // found the first difference at (ait, bit)

    if (ait == alast)
    {
        if (__builtin_expect(bit == blast, 1))
        {
            return (Intermediate) equal();
        }
        else
        {
            return (Intermediate) less();
        }
    }
    else if (bit == blast)
    {
        assert(ait != alast);
        return (Intermediate) greater();
    }
    else
    {
        assert(*ait != *bit);
        return comp(*ait, *bit);
    }
}
#endif

} // namespace sequence
} // namespace comparison

#endif



