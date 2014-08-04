// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file 
 * Constructing messages of limited length (e.g., for
 * exceptions)
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_COMPOUND_MESSAGE_H
#define COHORS_TYPES_COMPOUND_MESSAGE_H

#include "types/string.h"
#include "types/typeinfo.h"

namespace types {

namespace compound_message_ {

#if 0
template<class T, class Enable = void>
struct normalize 
{
  using type = T;
};

template<class T>
struct normalize <
  T, 
  typename std::enable_if<
    std::is_scalar<
      typename std::remove_reference<T>::type
    >::value
  >::type
>
{
  using type = typename std::decay<T>::type;
};
#endif

template<class... Args>
struct len_t;

template<class OutIt, int idx, class... Args>
class stringifier_t;

// an empty tail case
template<>
struct len_t<>
{
  static constexpr size_t max_length = 0;
};

template<class OutIt, int idx>
class stringifier_t<OutIt, idx>
{
public:
  void stringify(OutIt out, std::ios_base&) const noexcept
  {}
};

// for a string literal
template<class CharT, size_t N>
struct len_t<const CharT(&)[N]>
{
  static constexpr size_t max_length = N - 1;
};

template<class OutIt, int idx, size_t N>
class stringifier_t<
  OutIt,
  idx,
  const typename OutIt::char_type(&)[N]
>
{
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(const char_type(&s)[N]) noexcept
    : ptr(s) 
  {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      std::copy(ptr, ptr + N - 1, out);
    }
    catch(...) {
      *out++ = '*';
    }
  }

protected:
  const char_type *const ptr;
};

// for a basic_meta_string
template<class CharT, class Traits, CharT... CS>
struct len_t<basic_meta_string<CharT, Traits, CS...>&&>
{
  static constexpr size_t max_length = 
    basic_meta_string<CharT, Traits, CS...>::size();
};

template<
  class OutIt,
  int idx,
  class CharT,
  class Traits, 
  CharT... CS
>
class stringifier_t<
  OutIt,
  idx,
  basic_meta_string<CharT, Traits, CS...>&&
>
{
  using string = basic_meta_string<CharT, Traits, CS...>;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string) noexcept {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      const std::string s = string();
      std::copy(s.begin(), s.end(), out);
    }
    catch(...) {
      *out++ = '*';
    }
  }
};

// for basic_auto_string
template<class CharT, int16_t N, class Traits>
struct len_t<const basic_auto_string<CharT, N, Traits>&>
{
  static constexpr size_t max_length = (size_t) N;
};

template<class CharT, int16_t N, class Traits>
struct len_t<basic_auto_string<CharT, N, Traits>&&>
{
  static constexpr size_t max_length = (size_t) N;
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  int16_t N, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  const basic_auto_string<CharT, N, Traits>&
>
{
  using string = basic_auto_string<CharT, N, Traits>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '*';
    }
  }
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  int16_t N, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  basic_auto_string<CharT, N, Traits>&&
>
{
  using string = basic_auto_string<CharT, N, Traits>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '*';
    }
  }
};

#define COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(type)  \
template<>                                              \
struct len_t<type>                                      \
{                                                       \
  static constexpr size_t max_length =                  \
    std::numeric_limits<type>::digits10                 \
    + 1 /*possible sign*/;                              \
};                                                      \
                                                        \
template<class OutIt, int idx>                          \
class stringifier_t<OutIt, idx, type>                   \
{                                                       \
public:                                                 \
  using char_type = typename OutIt::char_type;          \
                                                        \
  stringifier_t(type v) noexcept : val(std::move(v)) {}            \
                                                        \
  void stringify(OutIt out, std::ios_base& st)          \
    const noexcept                                      \
  {                                                     \
    try {                                               \
      using namespace std;                              \
      use_facet<num_put<char_type, OutIt>>(st.getloc()) \
        . put(out, st, ' ', (long long) val);           \
    }                                                   \
    catch (...) {                                       \
      *out++ = '*';                                     \
    }                                                   \
  }                                                     \
                                                        \
protected:                                              \
  type val;                                       \
};

#define COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(type) \
template<>                                              \
struct len_t<type>                                      \
{                                                       \
  static constexpr size_t max_length =                  \
    std::numeric_limits<type>::digits10;                \
};                                                      \
                                                        \
template<class OutIt, int idx>                          \
class stringifier_t<OutIt, idx, type>                   \
{                                                       \
public:                                                 \
  using char_type = typename OutIt::char_type;          \
                                                        \
  stringifier_t(type v) noexcept : val(std::move(v)) {}            \
                                                        \
  void stringify(OutIt out, std::ios_base& st)          \
    const noexcept                                      \
  {                                                     \
    try {                                               \
      using namespace std;                              \
      use_facet<num_put<char_type, OutIt>>(st.getloc()) \
        . put(out, st, ' ', (unsigned long long) val);  \
    }                                                   \
    catch (...) {                                       \
      *out++ = '*';                                     \
    }                                                   \
  }                                                     \
                                                        \
protected:                                              \
  type val;                                       \
};

//COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(short);
//COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(int);
//COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long);
//COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long long);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const short&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const int&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const long&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const long long&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(short&&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(int&&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long&&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long long&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(const unsigned short&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(const unsigned int&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(const unsigned long&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(const unsigned long long&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(unsigned short&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(unsigned int&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(unsigned long&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(unsigned long long&&);

// for long double
// TODO enable_if(type class)
template<>
struct len_t<long double&>
{
  static constexpr size_t max_length = 
    std::numeric_limits<long double>::digits10 // mantissa
    + constexpr_string("-1.e-123").size();
};

template<class OutIt, int idx>
class stringifier_t<OutIt, idx, long double&>
{
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(long double v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base& st) 
    const noexcept
  {
    try {
      using namespace std;
      use_facet<num_put<char_type, OutIt>>(st.getloc())
        . put(out, st, ' ', val);
    }
    catch (...) {
      *out++ = '*';
    }
  }

protected:
  const long double val;
};

// for types::type
// TODO general case for types with operator auto_string()
template<class Type, int16_t MaxLen>
struct len_t<type<Type, MaxLen>&&>
{
  static constexpr size_t max_length = MaxLen;
};

template<class OutIt, int idx, class Type, int16_t MaxLen>
class stringifier_t<OutIt, idx, type<Type, MaxLen>&&>
{
  using typeinfo = type<Type, MaxLen>;
public:
  using char_type = char;

  stringifier_t(typeinfo) noexcept {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      const auto s = (auto_string<MaxLen>) typeinfo();
      std::copy(s.begin(), s.end(), out);
    }
    catch(...) {
      *out++ = '*';
    }
  }
};

// recursive
template<class Arg0, class... Args>
struct len_t<Arg0, Args...> : len_t<Arg0>, len_t<Args...>
{
  using head = len_t<Arg0>;
  using tail = len_t<Args...>;

  static constexpr size_t max_length = 
    head::max_length + tail::max_length;
};

template<class OutIt, int idx, class Arg0, class... Args>
class stringifier_t<OutIt, idx, Arg0, Args...>
  : public stringifier_t<
      OutIt, 
      idx,
#if 0
      typename normalize<Arg0>::type
#else
      Arg0
#endif
    >,
    public stringifier_t<
      OutIt, 
      idx + 1,
#if 0
      typename normalize<Args>::type...
#else
      Args...
#endif
    >
{
public:
  using head = stringifier_t <
    OutIt, 
    idx,
#if 0
    typename normalize<Arg0>::type
#else
    Arg0
#endif
  >;
  using tail = stringifier_t <
    OutIt, 
    idx + 1,
#if 0
    typename normalize<Args>::type...
#else
    Args...
#endif
  >;

#if 1
  stringifier_t(Arg0&& arg0, Args&&... args) noexcept
    : head(
        std::forward<Arg0>(arg0)
      ), 
      tail(
        std::forward<Args>(args)...
      )
  {}
#else
  stringifier_t(Arg0 arg0, Args... args) noexcept
    : head(arg0), 
      tail(args...)
  {}
#endif

  void stringify(OutIt out, std::ios_base& st) 
    const noexcept
  {
    head::stringify(out, st);
    tail::stringify(out, st);
  }
};

} // compound_message_

template<class... Args>
constexpr size_t compound_message_max_length()
{
  return compound_message_::len_t<
#if 0
    typename compound_message_::normalize<Args>::type...
#else
    Args...
#endif
  >::max_length;
}

template<class OutIt, class... Args>
using compound_message_t = 
  compound_message_::stringifier_t<OutIt, 0, Args...>;

template<class OutIt, class... Args>
auto compound_message(Args&&... args)
  -> compound_message_t<OutIt, Args&&...>
{
  return compound_message_t<OutIt, Args&&...>
    (std::forward<Args>(args)...);
}

} // types

#endif

