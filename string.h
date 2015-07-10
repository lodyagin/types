// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Different string types.
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

#ifndef TYPES_STRING_H
#define TYPES_STRING_H

//#include <iostream>
//#include <streambuf>
//#include <string>
#include <array>
#include <cstdint>
#include <limits>
//#include <bits/silent_assert.h>
//#include <bits/iterator.h>

namespace types {

// TODO move to some right place
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

namespace iterators_ {

struct begin_t {};
struct end_t {};

template<
  class CharT, 
  class Pointer, 
  class Reference
>
class safe_string
{
public:
  using iterator_category = 
    std::random_access_iterator_tag;
  using value_type = CharT;
  using difference_type = std::int16_t;
  using size_type = std::uint16_t;
  using pointer = Pointer;
  using reference = Reference;
  using const_pointer = const CharT*;
  using const_reference = const CharT&;

  template<class, std::int16_t, class>
  friend class basic_auto_string;

  template<class C, class P, class R>
  friend class safe_string;

  bool operator==(safe_string o) const noexcept
  {
    _silent_assert(base == o.base);
    return virtual_ptr() == o.virtual_ptr();
  }

  bool operator!=(safe_string o) const noexcept
  {
    return !this->operator==(o);
  }

  bool operator<(safe_string o) const noexcept
  {
    return *this - o < 0;
  }

  bool operator>=(safe_string o) const noexcept
  {
    return *this - o >= 0;
  }

  bool operator>(safe_string o) const noexcept
  {
    return *this - o > 0;
  }

  bool operator<=(safe_string o) const noexcept
  {
    return *this - o <= 0;
  }

  reference operator*() noexcept
  {
    return base[idx];
  }

  const_reference operator*() const noexcept
  {
    return base[idx];
  }

  safe_string& operator++() noexcept
  {
    if (__builtin_expect(idx++ >= n, 0))
    {
      idx = 0;
      ovf += n;
    }
    return *this;
  }

  safe_string operator++(int) noexcept
  {
    safe_string copy(*this);
    ++(*this);
    return copy;
  }

  difference_type operator-(safe_string o) const noexcept
  {
    _silent_assert(base == o.base);
    return virtual_ptr() - o.virtual_ptr();
  }

  // cast to const_iterator
  operator 
  safe_string<CharT, const_pointer, const_reference>() const 
    noexcept
  {
    using const_iterator = 
      safe_string<CharT, const_pointer, const_reference>;
    return const_iterator(base, n, idx, ovf);
  }

  static_assert(
    sizeof(size_type) < sizeof(std::size_t),
    "unable to correctly implement virtual_ptr()"
  );
  const_pointer virtual_ptr() const noexcept
  {
    return base + idx + ovf;
  }

  //protected:  //TODO problem with friend basic_auto_string in clang
  safe_string(
    pointer base_, 
    std::int16_t n_, 
    size_type idx_, 
    std::int16_t ovf_
  )  noexcept 
    : base(base_), idx(idx_), ovf(ovf_), n(n_) 
  {
    _silent_assert(n >= 0);
  }

  safe_string(pointer base_, std::int16_t n_, begin_t) 
	 noexcept 
    : safe_string(base_, n_, 0, 0)
  {}

  safe_string(pointer base_, std::int16_t n_, end_t) 
	 noexcept 
    : safe_string(base_, n_, 0, n_)
  {}

  pointer base;
  size_type idx;
  std::int16_t ovf;
  std::int16_t n;
};

} // iterators_

template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
struct basic_auto_string_traits
{
  using iterator = iterators_::safe_string
    <CharT, CharT*, CharT&>;
  using const_iterator = iterators_::safe_string
    <CharT, const CharT*, const CharT&>;
};

using auto_string_traits = basic_auto_string_traits<char>;
using auto_wstring_traits = 
  basic_auto_string_traits<wchar_t>;

//! A string with an automatic (cycled) storage.
//! It maintains two sizes: size of the buffer and size of
//! the string (the used part of the buffer). 
//! So, size() == end() - begin()
//! buf_size() - 1 = buf_end() - begin()
//! end() can be greater than buf_end() (it's cycled).

template <
  class CharT,
  std::int16_t N,
  class Traits = std::char_traits<CharT>
> 
class basic_auto_string 
{
  static_assert(
    N > 0, 
    "types::basic_auto_string: invalid size"
  );

/*
protected:
  using std_string = std::basic_string<CharT, Traits>;

  using std_string_const_iterator = typename
    std_string::const_iterator;
*/

public:
  using traits_type = Traits;
  using value_type = CharT;
  using size_type = std::uint16_t;
  using difference_type = std::int16_t;
  using iterator = typename 
    basic_auto_string_traits<CharT, Traits>::iterator;
  using const_iterator = typename 
   basic_auto_string_traits<CharT, Traits>
     ::const_iterator;

  basic_auto_string() noexcept : cur_end(begin()) 
  {
    m[N-1] = 0;
  }

  basic_auto_string(const CharT(&str)[N]) noexcept
    : cur_end(end())
  {
    traits_type::copy(m.data(), str, N);
  }

  basic_auto_string(const CharT* str) noexcept
    : cur_end(begin())
  {
    int i = 0;
    for (const CharT* src = str; 
         i < N && *src != 0; 
         ++i, ++src, ++cur_end
         )
      *cur_end = *src;

    if (i < N)
      *++cur_end = 0;
  }

  template<class Iterator>
  basic_auto_string(
    Iterator bg,
    Iterator en
  ) noexcept
    : cur_end(begin())
  {
    int i = 0;
    for (auto src = bg; 
         i < N && src < en; 
         ++i, ++src, ++cur_end
         )
      *cur_end = *src;

    if (i < N)
      *++cur_end = 0;
  }

  basic_auto_string(
    const std::basic_string<CharT, Traits>& str
  ) noexcept
    : basic_auto_string(str.begin(), str.end())
  {
  }

  void swap(basic_auto_string& o) noexcept
  {
    m.swap(o.m);
  }

  //! Returns the size of buffer with ending 0, so
  constexpr size_type buf_size() const 
  { 
    return N; 
  }

  size_type size() const 
  {
    return end() - begin();
  }

  iterator begin() noexcept
  {
    return iterator(m.data(), N-1, iterators_::begin_t());
  }

  const_iterator begin() const noexcept
  {
    return const_iterator(
      m.data(), N-1, iterators_::begin_t()
    );
  }

  iterator end() noexcept 
  {
    return cur_end;
  }

  const_iterator end() const noexcept
  {
    return cur_end;
  }

  //! Returns the end of buffer, not only filled size
  iterator buf_end() noexcept 
  {
    return iterator(m.data(), N-1, iterators_::end_t());
  }

  const_iterator buf_end() const noexcept
  {
    return const_iterator(
      m.data(), N-1, iterators_::end_t()
    );
  }

  value_type* data()
  {
    _silent_assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return m.data();
  }

  const value_type* data() const
  {
    _silent_assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return m.data();

    // _silent_assert(*end() == 0);
    // *end() = 0;
    // Don't do it, you can break a message which overruns
    // the buffer.
  }

  const value_type* c_str() const
  {
    _silent_assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return data();
  }

  void push_back(value_type ch) noexcept
  {
    *cur_end++ = ch;
  }

protected:
  // it is mutable - padding with '\0' is allowed
  mutable std::array<CharT, N> m;

  iterator cur_end;
};

// TODO
template<
  class CharT, 
  class Traits
>
class basic_auto_string<CharT, 0, Traits>;

template<std::int16_t N>
using auto_string = basic_auto_string<char, N>;

template<std::int16_t N>
using auto_wstring = basic_auto_string<wchar_t, N>;

#if 0
template <
  class CharT,
  std::int16_t N,
  class Traits = std::char_traits<CharT>
>
class basic_auto_stringbuf 
  : public std::basic_streambuf<CharT, Traits>
{
  typedef std::basic_streambuf<CharT, Traits> parent;

public:
  typedef CharT char_type;
  typedef Traits traits_type;
  typedef typename Traits::int_type int_type;
  typedef typename Traits::pos_type pos_type;
  typedef typename Traits::off_type off_type;
  typedef basic_auto_string<CharT, N, Traits> string;

  // TODO open modes
  basic_auto_stringbuf() 
  {
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size());
    this->setp(p, p + s.size());
  }

  // TODO open modes
  basic_auto_stringbuf(const CharT(&str)[N]) 
    : s(str) 
  {
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size());
    this->setp(p, p + s.size());
  }
  
  // TODO open modes
  basic_auto_stringbuf(const string& s_) 
  {
    str(s_);
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size());
    this->setp(p, p + s.size());
  }

  string& str() noexcept 
  {
    return s; 
  }

  const string& str() const noexcept 
  { 
    return s; 
  }

  void str(const string& s_) noexcept 
  {
    s = s_;
    auto* p = const_cast<char_type*>(s.data());
    this->setg(p, p, p + s.size()); // open modes ?
    this->setp(p, p + s.size());
  }

protected:
  // put area

  int_type overflow(int_type ch = Traits::eof()) override
  {
    if (Traits::eq_int_type(ch, Traits::eof()))
      return ch;

    if (s.end() < s.buf_end()) {
      s.push_back(ch);
      return ch;
    }
    else
      return Traits::eof();
  }

  // get area

  std::streamsize showmanyc() override
  {
    return this->egptr() - this->gptr();
  }

  int_type underflow() override
  {
    return (showmanyc()) 
      ? Traits::to_int_type(*this->gptr()) 
      : Traits::eof();
  }

  // positioning

  pos_type seekoff
    ( 
      off_type off, 
      std::ios_base::seekdir dir,
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    using namespace std;
    const pos_type end_pos = this->egptr()- this->eback();
    off_type abs_pos(0);

    switch((std::uint32_t)dir) {
      case ios_base::beg: 
        abs_pos = off;
        break;
      case ios_base::end:
        abs_pos = end_pos + off;
        break;
      case ios_base::cur:
        abs_pos = this->gptr() - this->eback() + off;
        break;
    }

    if (!(bool) abs_pos || abs_pos < 0) 
      // the rest will be checked in seekpos
      return pos_type(off_type(-1));
    
    return seekpos((off_type) abs_pos);
  }

  pos_type seekpos
    ( 
      pos_type pos, 
      std::ios_base::openmode which = std::ios_base::in
     ) override
  {
    const pos_type end_pos = this->egptr()- this->eback();

    if (pos > end_pos || which & std::ios_base::out)
      return pos_type(off_type(-1));

    this->setg
      (this->eback(), this->eback() + pos, this->egptr());
    return pos;
  }

  string s;
};

template<std::int16_t N>
using auto_stringbuf = basic_auto_stringbuf<char, N>;

template<std::int16_t N>
using auto_wstringbuf = basic_auto_stringbuf<wchar_t, N>;
#endif

/**
 * Just basic_constexpr_string. It is used for "wrap"
 * string literals and not pass strings with unpredicted
 * length to other functions (e.g., streams).
 */
template <
  class CharT,
  class Traits = std::char_traits<CharT>
> 
class basic_constexpr_string 
{
public:
  typedef std::uint32_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  template<std::uint32_t N>
  constexpr basic_constexpr_string(const char(&str)[N])
    noexcept
    : len(N-1), arr(str)
  {
  }

  constexpr basic_constexpr_string() noexcept
    : basic_constexpr_string("")
  {}

  constexpr size_type size() const noexcept { return len; }

  constexpr const value_type* data() const noexcept
  { 
    return arr; 
  }

  constexpr const value_type* c_str() const noexcept
  { 
    return arr; 
  }

  const value_type* begin() const noexcept
  {
    return arr;
  }

  const value_type* end() const noexcept
  {
    return arr + len;
  }

  operator std::basic_string<CharT, Traits>() const
  {
    return std::string(arr, len);
  }

  template<std::int16_t N>
  operator basic_auto_string<CharT, N, Traits>() const
  {
    return basic_auto_string<CharT, N, Traits>(
      arr, arr + len
    );
  }

  bool is_identical(basic_constexpr_string o) const
    noexcept
  {
    return arr == o.arr && len == o.len;
  }

  bool operator==(basic_constexpr_string o) const noexcept
  {
    return is_identical(o)
      || (len == o.len
          && traits_type::compare(arr, o.arr, len) == 0
          );
  }

  bool operator!=(basic_constexpr_string o) const noexcept
  {
    return !operator==(o);
  }

  bool operator<(basic_constexpr_string o) const noexcept
  {
    if (is_identical(o))
      return false;

    const int res = traits_type::compare(
      arr,
      o.arr,
      std::min(len, o.len)
    );
    if (__builtin_expect(res == 0, 0))
      return len < o.len;
    else
      return res < 0;
  }

  bool operator>(basic_constexpr_string o) const noexcept
  {
    if (is_identical(o))
      return false;

    const int res = traits_type::compare(
      arr,
      o.arr,
      std::min(len, o.len)
    );
    if (__builtin_expect(res == 0, 0))
      return len > o.len;
    else
      return res > 0;
  }

  bool operator<=(basic_constexpr_string o) const noexcept
  {
    return !operator>(o);
  }

  bool operator>=(basic_constexpr_string o) const noexcept
  {
    return !operator<(o);
  }

private:
  /*const*/ size_type len;
  const value_type* /*const*/ arr;
};

typedef basic_constexpr_string<char> constexpr_string;
typedef basic_constexpr_string<wchar_t> constexpr_wstring;

#if 0
template<class CharT, class Traits>
std::basic_ostream<CharT, Traits>& operator<<(
  std::basic_ostream<CharT, Traits>& out, 
  basic_constexpr_string<CharT, Traits> str
)
{
  return out.write(str.data(), str.size());
}
#endif

/**
 * It is usefull for parsing template literal operators.
 */
template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  CharT...
> 
class basic_meta_string;

template <
  class CharT,
  class Traits
> 
class basic_meta_string<CharT, Traits>
{
public:
  typedef std::uint16_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  constexpr static size_type size() { return 0; }

  operator std::string() const
  {
    return std::string();
  }

  template<class OutputIt>
  static void copy_to(OutputIt out)
  {
  }
};

template <
  class CharT,
  class Traits,
  CharT C0,
  CharT... CS
> 
class basic_meta_string<CharT, Traits, C0, CS...>
  : public basic_meta_string<CharT, Traits, CS...>
{
  using parent = basic_meta_string<CharT, Traits, CS...>;
public:
  typedef std::uint16_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  constexpr static size_type size()
  { 
    return parent::size() + 1;
  }

#if 0
  operator std::string() const
  {
    std::string res(size(), '\0');
    copy_to(res.begin());
    return res;
  }
#endif

  //! Copy the string to the output iterator
  template<class OutputIt>
  static void copy_to(OutputIt out)
  {
    *out++ = C0;
    parent::copy_to(out);
  }
};

template<char... cs>
using meta_string = basic_meta_string
  <char, std::char_traits<char>, cs...>;

template<wchar_t... wcs>
using meta_wstring = basic_meta_string
  <wchar_t, std::char_traits<wchar_t>, wcs...>;

} // types

#endif
