// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * This is a fixed std::pair, see
 * http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3680.html
 *
 * Public domain.
 */

#ifndef TYPES_PAIR_H
#define TYPES_PAIR_H

#include <type_traits>
#include <utility>
#include <bits/type_traits.h>

namespace types
{

template <std::size_t...> struct __tuple_indices {};

template <class _IdxType, _IdxType... _Values>
struct __integer_sequence {
  template <template <class _OIdxType, _OIdxType...> class _ToIndexSeq, class _ToIndexType>
  using __convert = _ToIndexSeq<_ToIndexType, _Values...>;

  template <std::size_t _Sp>
  using __to_tuple_indices = __tuple_indices<(_Values + _Sp)...>;
};


template <class T1, class T2>
struct pair 
{
    typedef T1 first_type;
    typedef T2 second_type;

    T1 first;
    T2 second;

    pair(const pair&) = default;
    pair(pair&&) = default;

    constexpr pair() : first(), second() 
    {
    }

    template<
        class U1 = T1, 
        class U2 = T2,
        typename std::enable_if<
               std::is_copy_constructible<U1>::value 
            && std::is_copy_constructible<U2>::value 
            && std::is_convertible<const U1&, U1>::value 
            && std::is_convertible<const U2&, U2>::value,
            bool
         >::type = false
    >
    constexpr pair(const T1& x, const T2& y) : first(x), second(y)
    {
    }

    template<
        class U1 = T1, 
        class U2 = T2,
        typename std::enable_if<
               std::is_copy_constructible<U1>::value 
            && std::is_copy_constructible<U2>::value 
            && !(   std::is_convertible<const U1&, U1>::value 
                 && std::is_convertible<const U2&, U2>::value),
            bool
         >::type = false
    >
    explicit constexpr pair(const T1& x, const T2& y) : first(x), second(y)
    {
    }

    template<
        class U, 
        class V,
        typename std::enable_if<
               std::is_constructible<first_type, U&&>::value 
            && std::is_constructible<second_type, V&&>::value 
            && std::is_convertible<U, first_type>::value 
            && std::is_convertible<V, second_type>::value,
            bool
         >::type = false
    > 
    constexpr pair(U&& x, V&& y)
        : first(std::forward<U>(x)), second(std::forward<V>(y))
    {
    }

    template<
        class U, 
        class V,
        typename std::enable_if<
               std::is_constructible<first_type, U&&>::value 
            && std::is_constructible<second_type, V&&>::value 
            && !(   std::is_convertible<U, first_type>::value 
                 && std::is_convertible<V, second_type>::value),
            bool
         >::type = false
    > 
    explicit constexpr pair(U&& x, V&& y)
        : first(std::forward<U>(x)), second(std::forward<V>(y))
    {
    }

    template<
        class U, 
        class V,
        typename std::enable_if<
               std::is_constructible<first_type, const U&>::value 
            && std::is_constructible<second_type, const V&>::value 
            && std::is_convertible<U, first_type>::value 
            && std::is_convertible<V, second_type>::value,
            bool
         >::type = false
    > 
    constexpr pair(const pair<U, V>& p) : first(p.first), second(p.second)
    {
    }

    template<
        class U, 
        class V,
        typename std::enable_if<
               std::is_constructible<first_type, const U&>::value 
            && std::is_constructible<second_type, const V&>::value 
            && !(   std::is_convertible<U, first_type>::value 
                 && std::is_convertible<V, second_type>::value),
            bool
         >::type = false
    > 
    explicit constexpr pair(const pair<U, V>& p) 
        : first(p.first), second(p.second)
    {
    }

    template<
        class U, 
        class V,
        typename std::enable_if<
               std::is_constructible<first_type, U&&>::value 
            && std::is_constructible<second_type, V&&>::value 
            && std::is_convertible<U, first_type>::value 
            && std::is_convertible<V, second_type>::value,
            bool
         >::type = false
    > 
    constexpr pair(pair<U, V>&& p)
        : first(std::forward<U>(p.first)),
          second(std::forward<V>(p.second))
    {
    }

    template<
        class U, 
        class V,
        typename std::enable_if<
               std::is_constructible<first_type, U&&>::value 
            && std::is_constructible<second_type, V&&>::value 
            && !(   std::is_convertible<U, first_type>::value 
                 && std::is_convertible<V, second_type>::value),
            bool
         >::type = false
    > 
    explicit constexpr pair(pair<U, V>&& p)
        : first(std::forward<U>(p.first)),
          second(std::forward<V>(p.second))
    {
    }

    pair& operator=(const pair& p)
    {
        first = p.first;
        second = p.second;
        return *this;
    }

    template<class U1, class U2>
    pair& operator=(const pair<U1, U2>& p)
    {
        first = p.first;
        second = p.second;
	return *this;
    }

    pair& operator=(pair&& p)
        noexcept(
            std::is_nothrow_move_assignable<first_type>::value
         && std::is_nothrow_move_assignable<second_type>::value
        )
      {
	first = std::forward<first_type>(p.first);
	second = std::forward<second_type>(p.second);
	return *this;
      }

    template<class U, class V> 
    pair& operator=(pair<U, V>&& p)
    {
        first = std::forward<U>(p.first);
        second = std::forward<V>(p.second);
        return *this;
    }

    void swap(pair& p)
        noexcept(
             noexcept(swap(first, p.first))
          && noexcept(swap(second, p.second))
        )
    {
        using std::swap;
        swap(first, p.first);
        swap(second, p.second);
    }

    template<class... Args1, class... Args2>
    pair(
        std::piecewise_construct_t, 
        std::tuple<Args1...> first, 
        std::tuple<Args2...> second
    )
        : pair(
            first, 
            second, 
            typename std::__make_tuple_indices<sizeof...(Args1)>::type(),
            typename std::__make_tuple_indices<sizeof...(Args2)>::type()
          )
    {
    }

private:
    template<class... Args1, std::size_t... Indexes1,
             class... Args2, std::size_t... Indexes2>
    pair(std::tuple<Args1...>& tuple1, std::tuple<Args2...>& tuple2,
	 std::__tuple_indices<Indexes1...>,
	 std::__tuple_indices<Indexes2...>
    )
      : first(std::forward<Args1>(std::get<Indexes1>(tuple1))...),
        second(std::forward<Args2>(std::get<Indexes2>(tuple2))...)
    { 
    }
};

template<class T1, class T2>
constexpr pair<typename std::__decay_and_strip<T1>::__type,
               typename std::__decay_and_strip<T2>::__type>
make_pair(T1&& x, T2&& y)
{
    typedef typename std::__decay_and_strip<T1>::__type first_t;
    typedef typename std::__decay_and_strip<T2>::__type second_t;
    typedef pair<first_t, second_t> pair_t;
    return pair_t(std::forward<T1>(x), std::forward<T2>(y));
}

} // types

#endif
