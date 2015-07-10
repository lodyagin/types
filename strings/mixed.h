// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
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

#ifndef TYPES_STRINGS_MIXED_H
#define TYPES_STRINGS_MIXED_H

namespace types {

/**
A string that has 3 modes, see copy_mode_type.
It is counter + pointer, can be not null-terminated.
\0 are legal symbols which take part in comparisons
("A\0\0" > "A\0").
*/
template<
  class CharT,
  bool null_terminated = true,
  class Traits = char_traits<CharT>,
  class Allocator = std::allocator<CharT>
>
class basic_mixed_string
{
public:
  typedef Traits traits_type;
  typedef typename traits_type::size_type size_type;

  enum copy_mode_type
  {
    deep, //< will copy the string content
    take_ownership, //< will copy just a pointer
    const_ptr //< will copy a pointer and never change 
              //<the string
  };

  //! Construct the string representing a literal
  template<size_type N>
  basic_mixed_string(const CharT(&s)[N]) : 
    cp_mode(const_ptr), allocator('fake')
  {
    init(const_cast<CharT*>(s), N - 1, N - 1);
  }

  basic_mixed_string(const CharT* s) : 
    cp_mode(const_ptr), allocator('fake')
  {
    ASSERT(s);
    const size_type len = traits_type::length(s);
    init(const_cast<CharT*>(s), len, len);
  }

#if 0
  // It is only for null_terminated == false case
  basic_mixed_string(const CharT* s, size_type count) :
    cp_mode(const_ptr), allocator('fake')
  {
    ASSERT(s);
    init(const_cast<CharT*>(s), count, count);
  }
#endif

  basic_mixed_string
    (CharT* s,
    copy_mode_type copy_mode,
    size_type reserved = 0, //!< 0 means reserved == count+sizeof(CharT)
    const Allocator& alloc = Allocator('stri')
    ) :
    cp_mode(copy_mode), allocator(alloc)
  {
    ASSERT(s);
    const size_type count = traits_type::length(s);
    init(s, count, reserved ? reserved : count);
  }

#if 0
  // It is only for null_terminated == false case
  basic_mixed_string
    (CharT* s,
    size_type count,
    copy_mode_type copy_mode,
    size_type reserved = 0, //!< 0 means reserved == count+sizeof(CharT)
    const Allocator& alloc = Allocator('stri')
    ) :
    cp_mode(copy_mode), allocator(alloc)
  {
    init(s, count, reserved ? reserved : count);
  }
#endif

  basic_mixed_string
    (UNICODE_STRING& s,
    copy_mode_type copy_mode,
    const Allocator& alloc = Allocator('stri'))
  :
    cp_mode(copy_mode), allocator(alloc)
  {
    ASSERT(s.Buffer);
    ASSERT(s.Length % sizeof(CharT) == 0);
    ASSERT(s.MaximumLength % sizeof(CharT) == 0);
    ASSERT(s.MaximumLength >= s.Length);

    // protect from null termination loss
    if (copy_mode != deep
      && null_terminated
      && (s.MaximumLength < s.Length + sizeof(CharT) 
          || s.Buffer[s.Length / sizeof(CharT)] != 0))
    {
      ASSERT(!is_valid_);
      ASSERT(false);
      return;
    }

    init(s.Buffer, s.Length / sizeof(CharT), s.MaximumLength / sizeof(CharT));
  }

protected:
  void init
    (CharT* s,
    size_type count,
    size_type reserved
    )
  {
    ASSERT(this);
    ASSERT(s);
    // check values initialized in ctr
    ASSERT(!is_valid_);

    if (reserved > traits_type::max_len 
        || reserved < count + (null_terminated ? 1 :0))
      return;

    // for exclude nullptr as a valid value for buf.Buffer
    static CharT dummy_buf[1] = { '\0' };

    CharT* str = dummy_buf;
    switch (cp_mode) {
    case deep:
      if (reserved > 0) {
        if ((str = allocator.allocate(reserved)) == nullptr)
          return;
        traits_type::copy(str, s, count);
        if (null_terminated)
          str[count] = 0;
      }
      break;
    case take_ownership:
    case const_ptr:
      str = s;
      if (null_terminated) {
        if (str[count] != 0) {
          ASSERT(!is_valid_);
          ASSERT(false);
          return;
        }
      }
      break;
    }
    // Do not use RtlInitUnicodeStringXXX to eliminate string iteration
    buf.Length = count * sizeof(CharT);
    buf.MaximumLength = reserved * sizeof(CharT);
    buf.Buffer = str;
    ASSERT(!null_terminated
      || (buf.MaximumLength >= buf.Length + sizeof(CharT) 
          && buf.Buffer[buf.Length / sizeof(CharT)] == 0));
    is_valid_ = true;
  }

public:
  basic_mixed_string(const basic_mixed_string& o) :
    is_valid_(o.is_valid_ && o.cp_mode == const_ptr),
    // neither deep copy string explicitly nor play with pointers
    buf(o.buf),
    cp_mode(o.cp_mode),
    allocator(o.allocator)
  {}

  basic_mixed_string(basic_mixed_string&& o) :
    is_valid_(o.is_valid_),
    buf(o.buf),
    cp_mode(o.cp_mode),
    allocator(o.allocator)
  {
    o.is_valid_ = false; // NB disable o.buf.Buffer deallocation
  }

  basic_mixed_string& operator= (basic_mixed_string o) //NB a copy constructor
  {
    swap(o); return *this;
  }

  basic_mixed_string& operator= (basic_mixed_string&& o)
  {
    swap(o); return *this;
  }

  ~basic_mixed_string()
  {
    if (is_valid_) {
      switch (cp_mode) {
      case deep:
        ASSERT(buf.Buffer);
        ASSERT(buf.MaximumLength % sizeof(CharT) == 0);
        allocator.deallocate(buf.Buffer, buf.MaximumLength / sizeof(CharT));
        break;
      case take_ownership:
      case const_ptr:
        break;
      }
    }
  }

  void swap(basic_mixed_string& o)
  {
    bits::swap(is_valid_, o.is_valid_);
    bits::swap(buf, o.buf);
    bits::swap(cp_mode, o.cp_mode);
  }

  basic_mixed_string deep_copy() const
  {
    ASSERT(buf.Length % sizeof(CharT) == 0);
    ASSERT(buf.MaximumLength % sizeof(CharT) == 0);
    return basic_mixed_string
      (buf.Buffer, buf.Length / sizeof(CharT), cp_mode, 
       buf.MaximumLength / sizeof(CharT), allocator);
  }

  bool operator==(const basic_mixed_string& s) const
  {
    ASSERT(buf.Length % sizeof(CharT) == 0);
    ASSERT(buf.MaximumLength % sizeof(CharT) == 0);
    return is_valid_ == s.is_valid_
      && buf.Length == s.buf.Length
      && (buf.Buffer == s.buf.Buffer 
          || traits_type::compare
                (buf.Buffer, s.buf.Buffer, buf.Length / sizeof(CharT)) == 0
          );
  }

  bool operator!=(const basic_mixed_string& s) const
  {
    return !operator==(s);
  }

  bool operator<(const basic_mixed_string& s) const
  {
    ASSERT(buf.Length % sizeof(CharT) == 0);
    ASSERT(buf.MaximumLength % sizeof(CharT) == 0);

    if (is_valid_ < s.is_valid_)
      return true;

    if (buf.Buffer == s.buf.Buffer) {
      if (buf.Length == s.buf.Length)
        return false;
      else
        return buf.Length < s.buf.Length;;
    }
    const int comp = traits_type::compare
      (buf.Buffer, s.buf.Buffer, min(buf.Length, s.buf.Length) / sizeof(CharT));
    if (comp == 0)
      return buf.Length < s.buf.Length;
    else
      return comp < 0;
  }


  bool operator>(const basic_mixed_string& s) const
  {
    ASSERT(buf.Length % sizeof(CharT) == 0);
    ASSERT(buf.MaximumLength % sizeof(CharT) == 0);

    if (is_valid_ > s.is_valid_)
      return true;

    if (buf.Buffer == s.buf.Buffer) {
      if (buf.Length == s.buf.Length)
        return false;
      else
        return buf.Length > s.buf.Length;;
    }
    const int comp = traits_type::compare
      (buf.Buffer, s.buf.Buffer, min(buf.Length, s.buf.Length) / sizeof(CharT));
    if (comp == 0)
      return buf.Length > s.buf.Length;
    else
      return comp > 0;
  }

  bool operator>=(const basic_mixed_string& s) const
  {
    return !operator<(s);
  }

  bool operator<=(const basic_mixed_string& s) const
  {
    return !operator>(s);
  }

  bool is_valid() const { return is_valid_; }

protected:
  //! To check whether the string is valid after construction
  bool is_valid_ = false;

  UNICODE_STRING buf;
  /*const*/ copy_mode_type cp_mode; // only swap can change it
  Allocator allocator;
};

//! _pn means allocation PagedPool and null terminated
typedef basic_mixed_string<WCHAR> wstring_pn;

} // types

#endif
