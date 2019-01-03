// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file 
 * Constructing messages of limited length (e.g., for
 * exceptions)
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

#ifndef TYPES_COMPOUND_MESSAGE_H
#define TYPES_COMPOUND_MESSAGE_H

#include <locale>
#include "types/string.h"
#include "types/traits.h"

namespace types {

enum class limit_policy { get_head, get_tail };

namespace compound_message_ {

//! A limiting wrapper for a type, it limits the output
//! field. 
template<
  std::int16_t MaxLen, 
  class T, 
  limit_policy lim_policy = limit_policy::get_head,
  typename T::value_type TruncationMark = '*'
>
struct limit_t
{
  static constexpr auto truncation_mark = 
    TruncationMark;

  limit_t(const T& orig_) : orig(orig_) {}
  const T& orig;
};

} // compound_message_

template<
  std::int16_t MaxLen, 
  limit_policy lim_policy = limit_policy::get_head, 
  class T = void
>
compound_message_::limit_t<MaxLen, T, lim_policy> 
limit(const T& orig)
{
  return compound_message_::limit_t
    <MaxLen, T, lim_policy>(orig);
}

template<std::int16_t MaxLen, class T>
auto limit_head(const T& orig)
  -> decltype(limit<MaxLen, limit_policy::get_tail>(orig))
{
  return limit<MaxLen, limit_policy::get_tail>(orig);
}

template<std::int16_t MaxLen, class T>
auto limit_tail(const T& orig)
  -> decltype(limit<MaxLen, limit_policy::get_head>(orig))
{
  return limit<MaxLen, limit_policy::get_head>(orig);
}

namespace compound_message_ {

template<int, class... Args>
struct len_t;

template<class OutIt, int idx, class... Args>
class stringifier_t;

// an empty tail case
template<int idx>
struct len_t<idx>
{
  static constexpr std::size_t max_length = 0;
};

template<class OutIt, int idx>
class stringifier_t<OutIt, idx>
{
public:
  void stringify(OutIt out, std::ios_base&) const noexcept
  {}
};

// for a string literal
template<int idx, class CharT, std::size_t N>
struct len_t<idx, const CharT(&)[N]>
{
  static constexpr std::size_t max_length = N - 1;
};

template<class OutIt, int idx, std::size_t N>
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
#if 0
    try {
      std::copy(ptr, ptr + N - 1, out);
    }
    catch(...) {
      *out++ = '?';
    }
#else
    std::copy(ptr, ptr + N - 1, out);
#endif
  }

protected:
  const char_type *const ptr;
};

#if 0
// for a basic_meta_string
template<int idx, class CharT, class Traits, CharT... CS>
struct len_t<
  idx, 
  basic_meta_string<CharT, Traits, CS...>&&
>
{
  static constexpr std::size_t max_length = 
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
      *out++ = '?';
    }
  }
};
#endif

// for basic_auto_string
template<int idx, class CharT, std::int16_t N, class Traits>
struct len_t<
  idx, 
  const strings::basic_auto_string<CharT, N, Traits>&
>
{
  static constexpr std::size_t max_length = (std::size_t) N;
};

template<int idx, class CharT, std::int16_t N, class Traits>
struct len_t<idx, strings::basic_auto_string<CharT, N, Traits>&&>
{
  static constexpr std::size_t max_length = (std::size_t) N;
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  std::int16_t N, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  const strings::basic_auto_string<CharT, N, Traits>&
>
{
  using string = strings::basic_auto_string<CharT, N, Traits>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
#if 0
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '?';
    }
#else
  std::copy(val.begin(), val.end(), out);
#endif
  }
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  std::int16_t N, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  strings::basic_auto_string<CharT, N, Traits>&&
>
{
  using string = strings::basic_auto_string<CharT, N, Traits>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
#if 0
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '?';
    }
#else
  std::copy(val.begin(), val.end(), out);
#endif
  }
};

// for basic_constexpr_string
template<int idx, class CharT, std::size_t MaxLen, class Traits>
struct len_t<
  idx, 
  const strings::basic_constexpr_string<CharT, Traits, MaxLen>
>
{
  static constexpr std::size_t max_length = MaxLen;
};

template<int idx, class CharT, std::size_t MaxLen, class Traits>
struct len_t<
  idx, 
  strings::basic_constexpr_string<CharT, Traits, MaxLen>&
>
{
  static constexpr std::size_t max_length = MaxLen;
};

template<int idx, class CharT, std::size_t MaxLen, class Traits>
struct len_t<
  idx, 
  const strings::basic_constexpr_string<CharT, Traits, MaxLen>&
>
{
  static constexpr std::size_t max_length = MaxLen;
};

template<int idx, class CharT, std::size_t MaxLen, class Traits>
struct len_t<idx, strings::basic_constexpr_string<CharT, Traits, MaxLen>&&>
{
  static constexpr std::size_t max_length = MaxLen;
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  std::size_t MaxLen, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  const strings::basic_constexpr_string<CharT, Traits, MaxLen>&
>
{
  using string = strings::basic_constexpr_string<CharT, Traits, MaxLen>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
#if 0
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '?';
    }
#else
  std::copy(val.begin(), val.end(), out);
#endif
  }
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  std::size_t MaxLen, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  strings::basic_constexpr_string<CharT, Traits, MaxLen>&
>
{
  using string = strings::basic_constexpr_string<CharT, Traits, MaxLen>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
#if 0
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '?';
    }
#else
  std::copy(val.begin(), val.end(), out);
#endif
  }
};

template<
  class OutIt, 
  int idx,
  class CharT, 
  std::size_t MaxLen, 
  class Traits
>
class stringifier_t<
  OutIt,
  idx,
  strings::basic_constexpr_string<CharT, Traits, MaxLen>&&
>
{
  using string = strings::basic_constexpr_string<CharT, Traits, MaxLen>;

  const string val;
public:
  using char_type = typename OutIt::char_type;

  stringifier_t(string v) noexcept : val(v) {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
#if 0
    try {
      std::copy(val.begin(), val.end(), out);
    }
    catch(...) {
      *out++ = '?';
    }
#else
  std::copy(val.begin(), val.end(), out);
#endif
  }
};

#define COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(Int)   \
template<int idx>                                       \
struct len_t<idx, Int>                                  \
{                                                       \
  static constexpr std::size_t max_length =             \
    std::numeric_limits<Int>::digits10 + 1              \
    + 1 /*possible sign*/;                              \
};                                                      \
                                                        \
template<class OutIt, int idx>                          \
class stringifier_t<OutIt, idx, Int>                    \
{                                                       \
public:                                                 \
  using char_type = typename OutIt::char_type;          \
                                                        \
  stringifier_t(Int v) noexcept : val(v) {}             \
                                                        \
  void stringify(OutIt out, std::ios_base& st)          \
    const noexcept                                      \
  {                                                     \
    std::num_put<char_type, OutIt>()                      \
        . put(out, st, ' ', (long long) val);           \
  }                                                     \
protected:                                              \
  typename std::remove_reference<Int>::type val;        \
};
/*
    try {                                               \
      std::use_facet<std::num_put<char_type, OutIt>>    \
        (st.getloc())   \
        . put(out, st, ' ', (long long) val);           \
    }                                                   \
    catch (...) {                                       \
      *out++ = '?';                                     \
    }                                                   \
  }                                                     \
                                                        \
protected:                                              \
  typename std::remove_reference<Int>::type val;                                       \
};
*/
#define COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT(Int) \
template<int idx>                                       \
struct len_t<idx, Int>                                  \
{                                                       \
  static constexpr std::size_t max_length =                  \
    std::numeric_limits<Int>::digits10 + 1;             \
};                                                      \
                                                        \
template<class OutIt, int idx>                          \
class stringifier_t<OutIt, idx, Int>                   \
{                                                       \
public:                                                 \
  using char_type = typename OutIt::char_type;          \
                                                        \
  stringifier_t(Int v) noexcept : val(v) {}            \
                                                        \
  void stringify(OutIt out, std::ios_base& st)          \
    const noexcept                                      \
  {                                                     \
    std::num_put<char_type, OutIt>()                    \
      . put(out, st, ' ', (unsigned long long) val);    \
  }                                                     \
protected:                                              \
  typename std::remove_reference<Int>::type val;        \
};
/*                                               \
    try {                                               \
      std::use_facet<std::num_put<char_type, OutIt>>    \
        (st.getloc()) \
        . put(out, st, ' ', (unsigned long long) val);  \
    }                                                   \
    catch (...) {                                       \
      *out++ = '?';                                     \
    }                                                   \
  }                                                     \
                                                        \
protected:                                              \
  typename std::remove_reference<Int>::type val;                                       \
};*/

COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(short&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(int&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long long&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const short&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const int&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const long&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(const long long&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(short&&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(int&&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long&&);
COHORS_TYPES_COMPOUND_MESSAGE_SIGNED_INT(long long&&);

COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned short&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned int&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned long&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned long long&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(const unsigned short&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(const unsigned int&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(const unsigned long&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(const unsigned long long&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned short&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned int&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned long&&);
COHORS_TYPES_COMPOUND_MESSAGE_UNSIGNED_INT\
(unsigned long long&&);

#if 0 // TODO numeric_limits in bare
// for long double
// TODO enable_if(type class)
template<int idx>
struct len_t<idx, long double&>
{
  static constexpr std::size_t max_length = 
    std::numeric_limits<long double>::max_digits10 
      // mantissa
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
      *out++ = '?';
    }
  }

protected:
  const long double val;
};
#endif

//! for limit_t<N>
template<
  int idx, 
  class Type, 
  std::int16_t MaxLen, 
  limit_policy LimPolicy
>
struct len_t<idx, limit_t<MaxLen, Type, LimPolicy>&&>
{
  static constexpr std::size_t max_length = MaxLen;
};

template<
  class OutIt, 
  int idx, 
  class Type, 
  std::int16_t MaxLen,
  limit_policy LimPolicy
>
class stringifier_t<
  OutIt, 
  idx, 
  limit_t<MaxLen, Type, LimPolicy>&&
>
{
public:
  using char_type = typename Type::value_type;

  stringifier_t(limit_t<MaxLen, Type, LimPolicy>&& lim) 
    noexcept 
    : t(lim.orig)
  {}

  void stringify(OutIt out, std::ios_base&) const noexcept
  {
    try {
      const bool trunc = t.begin() + MaxLen < t.end();
      if (!trunc)
        std::copy(t.begin(), t.end(), out);
      else {
        switch (LimPolicy) {
        case limit_policy::get_head:
          std::copy(
            t.begin(), 
            t.begin() + MaxLen - 1, 
            out
          );
          *out++ = limit_t<MaxLen, Type>::truncation_mark;
          break;
        case limit_policy::get_tail:
          *out++ = limit_t<MaxLen, Type>::truncation_mark;
          std::copy(
            t.end() - MaxLen + 1,
            t.end(),
            out
          );
          break;
        }
      }
    }
    catch(...) {
      *out++ = '?';
    }
  }

protected:
  Type t;
};

#if 0
// for types::type
// TODO general case for types with operator auto_string()
template<int idx, class Type, std::int16_t MaxLen>
struct len_t<idx, type<Type, MaxLen>&&>
{
  static constexpr std::size_t max_length = MaxLen;
};

template<class OutIt, int idx, class Type, std::int16_t MaxLen>
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
      *out++ = '?';
    }
  }
};
#endif

// recursive
template<int idx, class Arg0, class... Args>
struct len_t<idx, Arg0, Args...> 
  : len_t<idx, Arg0>, len_t<idx + 1, Args...>
{
  using head = len_t<idx, Arg0>;
  using tail = len_t<idx + 1, Args...>;

  static constexpr std::size_t max_length = 
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
constexpr std::size_t compound_message_max_length()
{
  return compound_message_::len_t<0,
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

