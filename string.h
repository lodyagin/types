// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file
 * Different string types.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_STRING_H
#define COHORS_TYPES_STRING_H

#include <ios>
#include <streambuf>
#include <string>
#include <array>
#include <cstdint>
#include <iterator>
#include <limits>
#include <assert.h>

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
  using difference_type = int16_t;
  using size_type = uint16_t;
  using pointer = Pointer;
  using reference = Reference;
  using const_pointer = const CharT*;
  using const_reference = const CharT&;

  template<class, int16_t, class>
  friend class basic_auto_string;

  template<class C, class P, class R>
  friend class safe_string;

  bool operator==(safe_string o) const noexcept
  {
    assert(base == o.base);
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
    assert(base == o.base);
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
    sizeof(size_type) < sizeof(size_t),
    "unable to correctly implement virtual_ptr()"
  );
  const_pointer virtual_ptr() const noexcept
  {
    return base + idx + ovf;
  }

  //protected:  //TODO problem with friend basic_auto_string in clang
  safe_string(
    pointer base_, 
    int16_t n_, 
    size_type idx_, 
    int16_t ovf_
  )  noexcept 
    : base(base_), idx(idx_), ovf(ovf_), n(n_) 
  {
    assert(n >= 0);
  }

  safe_string(pointer base_, int16_t n_, begin_t) noexcept 
    : safe_string(base_, n_, 0, 0)
  {}

  safe_string(pointer base_, int16_t n_, end_t) noexcept 
    : safe_string(base_, n_, 0, n_)
  {}

  pointer base;
  size_type idx;
  int16_t ovf;
  int16_t n;
};

} // iterators_

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
  typedef uint32_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  template<std::uint32_t N>
  constexpr basic_constexpr_string(const char(&str)[N])
    : len(N-1), arr(str)
  {
  }

  constexpr size_type size() const { return len; }

  constexpr const value_type* data() const 
  { 
    return arr; 
  }

  constexpr const value_type* c_str() const 
  { 
    return arr; 
  }

  const value_type* begin() const
  {
    return arr;
  }

  const value_type* end() const
  {
    return arr + len;
  }

private:
  const size_type len;
  const value_type* const arr;
};

typedef basic_constexpr_string<char> constexpr_string;
typedef basic_constexpr_string<wchar_t> constexpr_wstring;

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
  typedef uint16_t size_type;
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
  typedef uint16_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;

  constexpr static size_type size()
  { 
    return parent::size() + 1;
  }

  operator std::string() const
  {
    std::string res(size(), '\0');
    copy_to(res.begin());
    return res;
  }

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
  int16_t N,
  class Traits = std::char_traits<CharT>
> 
class basic_auto_string 
{
  static_assert(
    N > 0, 
    "types::basic_auto_string: invalid size"
  );

public:
  using traits_type = Traits;
  using value_type = CharT;
  using size_type = uint16_t;
  using difference_type = int16_t;
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

  basic_auto_string(const std::string str) noexcept
    : basic_auto_string(str.c_str())
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
    assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return m.data();
  }

  const value_type* data() const
  {
    assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return m.data();

    // assert(*end() == 0);
    // *end() = 0;
    // Don't do it, you can break a message which overruns
    // the buffer.
  }

  const value_type* c_str() const
  {
    assert(m[N-1] == 0);
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

template<int16_t N>
using auto_string = basic_auto_string<char, N>;

template<int16_t N>
using auto_wstring = basic_auto_string<wchar_t, N>;

template <
  class CharT,
  int16_t N,
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

    switch((uint32_t)dir) {
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

template<int16_t N>
using auto_stringbuf = basic_auto_stringbuf<char, N>;

template<int16_t N>
using auto_wstringbuf = basic_auto_stringbuf<wchar_t, N>;

} // types

#endif
