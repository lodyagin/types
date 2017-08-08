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
#include "types/traits.h"
#include "types/pair.h"

#ifndef BARE_CXX
#define _silent_assert assert
#endif

namespace types {

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
    if (__builtin_expect(++idx >= n, 0))
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

} // namespace types

namespace strings {

template<
  class CharT,
  class Traits = std::char_traits<CharT>
>
struct basic_auto_string_traits
{
  using iterator = types::iterators_::safe_string
    <CharT, CharT*, CharT&>;
  using const_iterator = types::iterators_::safe_string
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
  class Traits
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
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;
  using iterator = typename 
    basic_auto_string_traits<CharT, Traits>::iterator;
  using const_iterator = typename 
   basic_auto_string_traits<CharT, Traits>
     ::const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
/*
    if (i < N)
      *++cur_end = 0;
*/
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
/*
    if (i < N)
      *++cur_end = 0;
*/
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
    static_assert(N > 0, "N must be > 0");
    static_assert(sizeof(difference_type) >= sizeof(N), "difference_type is too small");
    return std::min(end() - begin(), static_cast<difference_type>(N-1));
  }

  size_type max_size() const
  {
    return buf_size() - 1;
  }

  bool overflow() const
  {
    return end() - begin() > max_size();
  }

  iterator begin() noexcept
  {
    return iterator(m.data(), N-1, types::iterators_::begin_t());
  }

  const_iterator begin() const noexcept
  {
    return const_iterator(
      m.data(), N-1, types::iterators_::begin_t()
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
    return iterator(m.data(), N-1, types::iterators_::end_t());
  }

  const_iterator buf_end() const noexcept
  {
    return const_iterator(
      m.data(), N-1, types::iterators_::end_t()
    );
  }

  void clear() noexcept
  {
//    m[0] = 0;
    cur_end = begin();
  }

  value_type* data()
  {
    _silent_assert(m[N-1] == 0);
    m[N-1] = 0; // for sure
    return m.data();
  }

  const value_type* data() const
  {
    const auto n = size();
    _silent_assert(n < N);
    m[n] = 0;
    return m.data();

    // _silent_assert(*end() == 0);
    // *end() = 0;
    // Don't do it, you can break a message which overruns
    // the buffer.
  }

  const value_type* c_str() const
  {
    return data();
  }

  void push_back(value_type ch) noexcept
  {
    if (__builtin_expect(size() < max_size(), 1))
    {
      *cur_end++ = ch;
    }
/* TODO
    else 
      throw length_error;
*/
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

#ifndef BARE_CXX
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
  class Traits,
  std::size_t MaxLen
> 
class basic_constexpr_string 
{
public:
  typedef std::uint32_t size_type;
  typedef CharT value_type;
  typedef Traits traits_type;
  typedef std::int32_t difference_type;
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef types::iterators_::safe_string<CharT, CharT*, CharT&> iterator;
  typedef types::iterators_::safe_string<CharT, const CharT*, const CharT&> const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  template<std::uint32_t N>
  constexpr basic_constexpr_string(const char(&str)[N])
    noexcept
    : len(N-1), arr(str)
  {
    static_assert(N <= MaxLen, "basic_constexpr_string MaxLen overflow");
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

  // TODO generic comparison
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
  size_type len;
  const_pointer arr;
};

using constexpr_string = basic_constexpr_string<char>;
using constexpr_wstring = basic_constexpr_string<wchar_t>;

template<std::size_t MaxLen>
using lim_constexpr_string = basic_constexpr_string<char, std::char_traits<char>, MaxLen>;

template<std::size_t MaxLen>
using lim_constexpr_wstring = basic_constexpr_string<wchar_t, std::char_traits<wchar_t>, MaxLen>;

static_assert(types::is_string<constexpr_string>::value, "is_string is failed for constexpr_string");
static_assert(types::is_string<constexpr_wstring>::value, "is_string is failed for constexpr_wstring");

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

/**
 * An STL-like string interface implementation, char[] version.
 * Use like this:
 *    const char buf[32];
 *    auto& str = string::adapter(buf);
 */
template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base,
    bool O1Size
>
class basic_const_char_array : public Base
{
public:
    typedef Traits traits_type;
    typedef CharT value_type;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef CharT& reference;
    typedef const CharT& const_reference;
    typedef CharT* pointer;
    typedef const CharT* const_pointer;
    typedef pointer iterator; // FIXME safe iterator
    typedef const_pointer const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    static constexpr size_type npos= std::numeric_limits<size_type>::max();
    static_assert(MaxSize < npos, "string size overlaps npos");

/*  [========[   basic_const_char_array - element access   ]=========]  */

#if 0 // wait out_of_range
    //! Returns a reference to the character at specified location
    //! pos. Bounds checking is performed, exception of type
    //! std::out_of_range will be thrown on invalid access.
    const_reference at(size_type n) const
    {
        if (n >= MaxSize)
        {
            throw std::out_of_range("basic_const_char_array::at()");
        }
        return operator[](n);
    }
#endif

    constexpr const_reference operator[](size_type n) const
    {
        return bs[n];
    }

    //! Returns reference to the first character in the string. The
    //! behavior is undefined if empty() == true.
    const_reference front() const
    {
        _silent_assert(!empty());
        return bs[0];
    }

    //! Returns reference to the last character in the string. The
    //! behavior is undefined if empty() == true.
    const_reference back() const
    {
        _silent_assert(!empty());
        return bs[size()-1];
    }

    //! Returns a pointer to a null-terminated character array with
    //! data equivalent to those stored in the string. The pointer is
    //! such that the range [c_str(); c_str() + size()] is valid and
    //! the values in it correspond to the values stored in the string
    //! with an additional null character after the last position.
    const CharT* c_str() const
    {
        if (MaxSize > 0)
        {
            this->do_zero_termination();
            return bs;
        }
        else
        {
            return "";
        }
    }

    //! The same as c_str().
    const CharT* data() const
    {
        return c_str();
    }

/*  [===========[   basic_const_char_array - iterators   ]===========]  */

    constexpr const_iterator begin() const
    {
        return bs;
    }
    
    constexpr const_iterator cbegin() const
    {
        return bs;
    }

    const_iterator end() const
    {
        return bs + size();
    }
    
    const_iterator cend() const
    {
        return bs + size();
    }

    const_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }
    
    const_iterator crbegin() const
    {
        return rbegin();
    }

    const_iterator rend() const
    {
        return const_reverse_iterator(begin());
    }
    
    const_iterator crend() const
    {
        return rend();
    }

/*  [===========[   basic_const_char_array - capacity   ]============]  */

    constexpr static bool has_o1_size = O1Size;

    //! Return the size of the string == 
    //! std::min(char_traits::length(s), max_size())
    //! NB it is not the same as the size of char array (MaxLength).
    size_type size() const
    {
        if (has_o1_size)
        {
            return max_size();
        }
        else
        {
            const std::size_t lim = max_size();
            if (lim == 0)
            {
                return 0;
            }
            this->do_zero_termination(); 
                // for char_traits::length() safety
            return std::min(lim, traits_type::length(bs));
        }
    }

    //! The same as size()
    size_type length() const
    {
        return size();
    }

    //! Returns the maximum number of elements the string is able to hold.
    //! Terminating 0 is not included.
    constexpr size_type max_size() const
    {
        return (MaxSize > 0) ? MaxSize - 1 : 0;
    }

    //! Checks if the string has no characters, i.e. whether begin()
    //! == end().
    bool empty() const
    {
        return size() == 0;
    }

    constexpr size_type capacity() const
    {
        return max_size();
    }

/*  [=======[   basic_const_char_array - operations   ]=======]  */


/*  [==========[   basic_const_char_array - comparison   ]===========]  */

#if 0 // waits enumerate
    //! Lexicographically compares this string to str. Uses traits_type.
    template<
        class Result = int,
        class String,
        typename std::enable_if<
            is_compatible<basic_const_char_array, String>::value,
            bool
        >::type = false
    >
    Result compare(const String& str) const noexcept
    {
        using namespace ::comparison;

        return sequence::compare<Result>(
            *this, 
            str, 
            character::compare<Result, value_type, traits_type>()
        );
    }

    /*template<std::size_t M>
    int compare(const char (&str)[M]) const noexcept
    {
        return compare(adapter(str));
    }*/

/*  [==========[   basic_const_char_array - comparison   ]===========]  */

    template<class String>
    bool operator==(const String& o) const noexcept
    {
        return compare<bool>(o);
    }

    template<class String>
    friend bool operator==(
        const String& a, 
        const basic_const_char_array& b
    ) noexcept
    {
        return b.operator==(a);
    }

    template<class String>
    bool operator!=(const String& o) const noexcept
    {
        return !operator==(o);
    }

    template<class String>
    friend bool operator!=(
        const String& a, 
        const basic_const_char_array& b
    ) noexcept
    {
        return b != a;
    }
#endif

/*  [============[   basic_const_char_array - search   ]=============]  */

#if 0 // waits std::search
    //! Finds the first substring equal to the given character
    //! sequence. Search begins at pos, i.e. the found substring must
    //! not begin in a position preceding pos.
    //! @return Position of the first character of the found substring
    //! or npos if no such substring is found.
    //!
    template<std::size_t M>
    size_type find(const char (&str) [M], size_type pos = 0) const noexcept
    {
        const auto& s = adapter(str);
        if (__builtin_expect(pos + s.size() >= size(), 0))
        {
            return npos;
        }

        const_iterator it = std::search(
            begin() + pos, 
            end(), 
            s.begin(), 
            s.end(),
            [](CharT a, CharT b)
            {
                return traits_type::eq(a, b);
            }
        );
        if (it == end())
        {
            return npos;
        }
        else
        {
            return it - begin();
        }
    }
#endif

    //! For testing purposes.
    bool __invariants() const
    {
        return this != nullptr
            && size() <= max_size()
            && (MaxSize == 0 || bs[MaxSize-1] == 0)
            && std::distance(begin(), end()) == size();
    }

protected:
    CharT bs[];

    basic_const_char_array() noexcept
    {
    }

    ~basic_const_char_array()
    {
    }

private:
    basic_const_char_array(const basic_const_char_array&);
    basic_const_char_array& operator=(const basic_const_char_array&);
};

template<
    std::size_t MaxSize,
    class CharT, 
    class Traits,
    class Base,
    bool O1Size
>
constexpr std::size_t 
basic_const_char_array<MaxSize, CharT, Traits, Base, O1Size>::npos;

/* ----- char[] (basic_char_array) adapter ----- */

template<
    class CharT,
    class Traits, //= std::char_traits<CharT>,
    std::size_t BuffSize,
    typename std::enable_if<!std::is_const<CharT>::value, bool>::type
    //= false
    >
basic_char_array<
    BuffSize, 
    CharT, 
    Traits, 
    zero_termination_::static_size::force<BuffSize, CharT>
>& 
//
adapter(CharT (&str)[BuffSize])
{
    typedef basic_char_array<
        BuffSize, 
        CharT, 
        Traits, 
        zero_termination_::static_size::force<BuffSize, CharT>
    > ret_string_t;

    assert(str);
    ret_string_t* p = reinterpret_cast<ret_string_t*>(str);
    p->do_zero_termination();
    return *p;
}

/* ----- const char[] (basic_const_char_array) adapter ----- */

template<
    class CharT,
    class Traits,
        // = std::char_traits<typename std::remove_const<CharT>::type>,
    std::size_t BuffSize,
    typename std::enable_if<std::is_const<CharT>::value, bool>::type
        //= false
    >
const basic_const_char_array<
    BuffSize, 
    typename std::remove_const<CharT>::type, 
    Traits, 
    zero_termination_::static_size::check<
        BuffSize, 
        typename std::remove_const<CharT>::type
    >,
    false // NB
>& 
//
adapter(CharT (&str)[BuffSize])
{
    typedef basic_const_char_array<
        BuffSize, 
        typename std::remove_const<CharT>::type, 
        Traits, 
        zero_termination_::static_size::check<
            BuffSize, 
            typename std::remove_const<CharT>::type
        >,
        false
    > ret_string_t;

    _silent_assert(str);
    const ret_string_t* p = reinterpret_cast<const ret_string_t*>(str);
    p->do_zero_termination();
    //assert(BuffSize == 0 || BuffSize-1 == Traits::length(str));
    //static_assert(arrays::has_o1_size<ret_string_t>::value, "error");
    return *p;
}

//! The length of the string constant is defined in compilation time
template<
    class CharT,
    class Traits=std::char_traits<CharT>,
    std::size_t N,
    typename std::enable_if<!std::is_const<CharT>::value, bool>::type
        = false
>
const basic_constexpr_string<CharT, Traits>
fast_constant(CharT (&str)[N])
{
    typedef basic_constexpr_string<CharT, Traits> ret_string_t;
    static_assert(arrays::has_o1_size<ret_string_t>::value, "error");
    return ret_string_t(str);
}
  
//! The length of the string constant is defined in compilation time
template<
    class CharT,
    class Traits=std::char_traits<typename std::remove_const<CharT>::type>,
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
    true //NB
>& 
//
fast_constant(CharT (&str)[MaxSize])
{
    typedef basic_const_char_array<
        MaxSize, 
        typename std::remove_const<CharT>::type, 
        Traits, 
        zero_termination_::static_size::check<
            MaxSize, 
            typename std::remove_const<CharT>::type
        >,
        true
    > ret_string_t;

    //_silent_assert(str);
    const ret_string_t* p = reinterpret_cast<const ret_string_t*>(str);
    //p->do_zero_termination();
    _silent_assert(MaxSize > 0);
    _silent_assert(MaxSize-1 == Traits::length(str));
    static_assert(arrays::has_o1_size<ret_string_t>::value, "error");
    return *p;
}

} // namespace strings

#endif

