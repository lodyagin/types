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

namespace types
{

// this part is copy-pasted from clang libcxx

template <std::size_t...> struct __tuple_indices {};

template <class _IdxType, _IdxType... _Values>
struct __integer_sequence {
  template <template <class _OIdxType, _OIdxType...> class _ToIndexSeq, class _ToIndexType>
  using __convert = _ToIndexSeq<_ToIndexType, _Values...>;

  template <std::size_t _Sp>
  using __to_tuple_indices = __tuple_indices<(_Values + _Sp)...>;
};

//#if !__has_builtin(__make_integer_seq) || defined(_LIBCPP_TESTING_FALLBACK_MAKE_INTEGER_SEQUENCE)
namespace __detail {

template<typename _Tp, std::size_t ..._Extra> struct __repeat;
template<typename _Tp, _Tp ..._Np, std::size_t ..._Extra> struct __repeat<__integer_sequence<_Tp, _Np...>, _Extra...> {
  typedef __integer_sequence<_Tp,
                           _Np...,
                           sizeof...(_Np) + _Np...,
                           2 * sizeof...(_Np) + _Np...,
                           3 * sizeof...(_Np) + _Np...,
                           4 * sizeof...(_Np) + _Np...,
                           5 * sizeof...(_Np) + _Np...,
                           6 * sizeof...(_Np) + _Np...,
                           7 * sizeof...(_Np) + _Np...,
                           _Extra...> type;
};

template<std::size_t _Np> struct __parity;
template<std::size_t _Np> struct __make : __parity<_Np % 8>::template __pmake<_Np> {};

template<> struct __make<0> { typedef __integer_sequence<std::size_t> type; };
template<> struct __make<1> { typedef __integer_sequence<std::size_t, 0> type; };
template<> struct __make<2> { typedef __integer_sequence<std::size_t, 0, 1> type; };
template<> struct __make<3> { typedef __integer_sequence<std::size_t, 0, 1, 2> type; };
template<> struct __make<4> { typedef __integer_sequence<std::size_t, 0, 1, 2, 3> type; };
template<> struct __make<5> { typedef __integer_sequence<std::size_t, 0, 1, 2, 3, 4> type; };
template<> struct __make<6> { typedef __integer_sequence<std::size_t, 0, 1, 2, 3, 4, 5> type; };
template<> struct __make<7> { typedef __integer_sequence<std::size_t, 0, 1, 2, 3, 4, 5, 6> type; };

template<> struct __parity<0> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type> {}; };
template<> struct __parity<1> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 1> {}; };
template<> struct __parity<2> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 2, _Np - 1> {}; };
template<> struct __parity<3> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 3, _Np - 2, _Np - 1> {}; };
template<> struct __parity<4> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 4, _Np - 3, _Np - 2, _Np - 1> {}; };
template<> struct __parity<5> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 5, _Np - 4, _Np - 3, _Np - 2, _Np - 1> {}; };
template<> struct __parity<6> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 6, _Np - 5, _Np - 4, _Np - 3, _Np - 2, _Np - 1> {}; };
template<> struct __parity<7> { template<std::size_t _Np> struct __pmake : __repeat<typename __make<_Np / 8>::type, _Np - 7, _Np - 6, _Np - 5, _Np - 4, _Np - 3, _Np - 2, _Np - 1> {}; };

} // namespace detail

//#endif  // !__has_builtin(__make_integer_seq) || defined(_LIBCPP_TESTING_FALLBACK_MAKE_INTEGER_SEQUENCE)

/*#if __has_builtin(__make_integer_seq)
template <std::size_t _Ep, std::size_t _Sp>
using __make_indices_imp =
    typename __make_integer_seq<__integer_sequence, std::size_t, _Ep - _Sp>::template
    __to_tuple_indices<_Sp>;
#else
*/
template <std::size_t _Ep, std::size_t _Sp>
using __make_indices_imp =
    typename __detail::__make<_Ep - _Sp>::type::template __to_tuple_indices<_Sp>;

//#endif


template <std::size_t _Ep, std::size_t _Sp = 0>
struct __make_tuple_indices
{
    static_assert(_Sp <= _Ep, "__make_tuple_indices input error");
    typedef __make_indices_imp<_Ep, _Sp> type;
};

// Helper which adds a reference to a type when given a reference_wrapper
template<typename _Tp>
struct __strip_reference_wrapper
{
    typedef _Tp __type;
};

template<typename _Tp>
struct __strip_reference_wrapper<std::reference_wrapper<_Tp> >
{
    typedef _Tp& __type;
};

template<typename _Tp>
struct __strip_reference_wrapper<const std::reference_wrapper<_Tp> >
{
    typedef _Tp& __type;
};

template<typename _Tp>
struct __decay_and_strip
{
    typedef typename __strip_reference_wrapper<
     typename std::decay<_Tp>::type>::__type __type;
};

// ---- end of copy-pasted part

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
            typename __make_tuple_indices<sizeof...(Args1)>::type(),
            typename __make_tuple_indices<sizeof...(Args2)>::type()
          )
    {
    }

private:
    template<class... Args1, std::size_t... Indexes1,
             class... Args2, std::size_t... Indexes2>
    pair(std::tuple<Args1...>& tuple1, std::tuple<Args2...>& tuple2,
	 __tuple_indices<Indexes1...>,
	 __tuple_indices<Indexes2...>
    )
      : first(std::forward<Args1>(std::get<Indexes1>(tuple1))...),
        second(std::forward<Args2>(std::get<Indexes2>(tuple2))...)
    { 
    }
};

template<class T1, class T2>
constexpr pair<typename __decay_and_strip<T1>::__type,
               typename __decay_and_strip<T2>::__type>
make_pair(T1&& x, T2&& y)
{
    typedef typename __decay_and_strip<T1>::__type first_t;
    typedef typename __decay_and_strip<T2>::__type second_t;
    typedef pair<first_t, second_t> pair_t;
    return pair_t(std::forward<T1>(x), std::forward<T2>(y));
}

} // types

#endif
