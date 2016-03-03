// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Arithmetic operations with overflow control
 *
 * This file (originally) was a part of public
 * https://github.com/lodyagin/types repository.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (c) 2013, Sergei Lodyagin
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


#ifndef TYPES_SAFE_H
#define TYPES_SAFE_H

#include <iostream>
#include <type_traits>
#include <stdexcept>
#include <limits>
#include <cmath>
#include "types/exception.h"
#include "types/typeinfo.h"

namespace types {

template<class UInt, class = void>
struct bits;

template<>
struct bits<unsigned int>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(unsigned i)
  {
    return i
      ? sizeof(i) * 8 - __builtin_clz(i)
      : 0;
  }
};

template<>
struct bits<unsigned long>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(unsigned long i)
  {
    return i
      ? sizeof(i) * 8 - __builtin_clzl(i)
      : 0;
  }
};

template<>
struct bits<unsigned long long>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(unsigned long long i)
  {
    return i
      ? sizeof(i) * 8 - __builtin_clzll(i)
      : 0;
  }
};

#if 0
template<>
struct bits<
  UInt,
  typename std::enable_if<
    std::is_integral<UInt>::value, 
    // have no sence for signed, and so
    // disabled to catch logic error in your program
    std::is_unsigned<UInt>::value
  >::type
>
{
  //! Calculate the number of highest bit in i starting from
  //! 1. Return 0 if i == 0.
  static unsigned highest_1(UInt i)
  {
  }
}
#endif

template <class UInt>
unsigned highest_bit1(UInt i)
{
#if 1 // using GCC builtin
  return bits<UInt>::highest_1(i);
#else
  int res;
  // TODO check with assembly
  for (res = 0; i != 0; i >>= 1, ++res)
    ;
  return res;
#endif
}

//! @exception overflow (for any safe<T> type)
struct overflow_error : virtual std::exception {};

//! Not defined for not integral and unsigned integral types
template <
  class Int, 
  bool = std::is_integral<Int>::value,
  bool = std::is_signed<Int>::value
>
class safe;

/**
  * It just an integer overflow checking wrapper for any
  * integral type. It accumulates the overflow in a flag to
  * appear later. There are two basic places where it will
  * appear: operator bool() which return the overflow as
  * true (no overflow) / false and cast to number which
  * will raise std::overflow_error. Usually you need to
  * check a result for overflow before casting this class
  * to number.
  *
  * Implementing ideas from 
  * http://www.fefe.de/intof.html and
  * https://www.securecoding.cert.org/confluence/display/seccode/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
  *
  *
  * @autor Sergei Lodyagin
  */
template<class Int>
class safe<Int, true, true>
{
  static_assert( std::is_signed<Int>::value, 
                 "modular_type must be signed" );

  template<class Int2, bool, bool>
  friend class safe;
public:
  static constexpr Int max = 
    std::numeric_limits<Int>::max();

  static constexpr Int min = 
    std::numeric_limits<Int>::min();

  //! @exception overflow for this safe<Int> type only
  struct overflow_error : types::overflow_error {};

  //! The default value is overflow
  constexpr safe() noexcept : no_ovf(false) {}

  constexpr safe(short av) noexcept
    : safe((Int) av, av == (Int) av)
  {}

  constexpr safe(unsigned short av) noexcept
    : safe((Int) av, av == (Int) av && (short) av >= 0)
  {}

  constexpr safe(int av) noexcept
    : safe((Int) av, av == (Int) av)
  {}

  constexpr safe(unsigned av) noexcept
    : safe((Int) av, av == (Int) av && (int) av >= 0)
  {}

  constexpr safe(long av) noexcept
    : safe((Int) av, av == (Int) av)
  {}

  constexpr safe(unsigned long av) noexcept
    : safe((Int) av, av == (Int) av && (long) av >= 0)
  {}

  constexpr safe(long long av) noexcept
    : safe(av, av == (Int) av) 
  {}

  constexpr safe(unsigned long long av) noexcept
    : safe(av, (long long) av == (Int) av && (long long) av >= 0)
  {}

  explicit constexpr safe(long double ld) noexcept
    : safe((Int) ld, std::fabs(ld - (Int) ld) < 0.5)
  {}

  safe(const safe& o) 
    : v(o.v), no_ovf(o.no_ovf), rem(o.rem)
  {}

  safe& operator = (Int av)
  {
    v = av;
    no_ovf = true;
    rem = false;
    return *this;
  }

  safe& operator = (const safe& o)
  {
    v = o.v;
    no_ovf = o.no_ovf;
    rem = o.rem;
    return *this;
  }

  //! Return an absolute value
  safe abs() const noexcept
  {
    safe copy(*this);
    if (copy.v < 0)
      copy.v = -copy.v;
    return copy;
  }

  /*
   * There is a solution based on a highest bits
   * calculation but it works for unsigned only. Checking
   * sign of argument is expensive because we will have
   * no jump prediction for such condition.
   */
  safe& operator += (safe sb) noexcept
  {
    Int& a = v;
    const Int& b = sb.v;

    // NB & not &&
    if (__builtin_expect((b > 0) & (max - b < a), 0)) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect((b < 0) & (min - b > a), 0)) {
      no_ovf = false;
      return *this;
    }

    a += b;
    inherit_status(sb);
    return *this;
  }

  safe& operator += (Int b) noexcept
  {
    return operator+=(safe(b));
  }

  safe& operator -= (safe b) noexcept
  {
    return operator+=(-b);
  }

  safe& operator -= (Int b) noexcept
  {
    return operator-=(safe(b));
  }

  template<class Int2>
  safe& operator *= (safe<Int2> b) noexcept
  {
    // TODO check what is faster
#if 1
    const typename std::make_unsigned<Int>::type ua = 
      std::abs(v);
    const typename std::make_unsigned<Int2>::type ub = 
      std::abs(b.v);

    if (__builtin_expect(
          highest_bit1(ua) + highest_bit1(ub) >
            ( std::is_signed<Int>::value 
                ? sizeof(Int) * 8 - 1 : sizeof(Int) * 8
            ),
          0
        ))
      no_ovf = false;
    else {
      v *= b.v;
      inherit_status(b);
    }
#else
    Int& a = v;
    const Int& b = sb.v;

    // NB & not &&
    if (__builtin_expect
         ((a > 0) & (b > 0) & (a > max / b), 0) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect
         ((a > 0) & (b <= 0) & (b < min / a), 0) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect
         ((a <= 0) & (b > 0) & (a < min / b), 0) {
      no_ovf = false;
      return *this;
    }
    if (__builtin_expect
         ((a < 0) & (b <= 0) & (b < max / a), 0) {
      no_ovf = false;
      return *this;
    }
    
    a *= b;
    inherit_status(sb);
#endif

    return *this;
  }

  template<class Int2>
  safe& operator *= (Int2 b) noexcept
  {
    return operator*=(safe<Int2>(b));
  }

  template<class Int2>
  safe& operator /= (safe<Int2> b) noexcept
  {
    Int copy = v;

    operator%=(b);
    // division overflow and overflow in b are alredy
    // checked in operator %=
  
    rem = rem || v != 0;
    v = copy / b.v; // NB no_ovf is the same
    
    return *this;
  }

  safe& operator /= (Int b) noexcept
  {
    return operator/=(safe(b));
  }

  template<class Int2>
  safe& operator %= (safe<Int2> b) noexcept
  {
    // it seams except for b == 0 this operation is always
    // safe.

    // division by zero is overflow
    if (__builtin_expect(b.v == 0, 0)) {
      no_ovf = false;
      return *this;
    }
    // is it needed? If no, move to /=
    if (__builtin_expect(v == min, 0)
        && __builtin_expect(b.v == -1, 0)) 
    {
      no_ovf = false;
      return *this;
    }
    

    v %= b.v;
    inherit_status(b);
    return *this;
  }

  safe& operator %= (Int b) noexcept
  {
    return operator%=(safe(b));
  }

  safe operator + () const noexcept
  {
    return *this;
  }

  safe operator - () const noexcept
  {
    using namespace std;
    return safe
      (-v, (std::abs(min) <= std::abs(max) || v != min)
       && (std::abs(max) <= std::abs(min) || v != max));
  }

  safe operator + (safe b) const noexcept
  {
    return b += *this;
  }

  safe operator - (safe b) const noexcept
  {
//    safe c(*this);
    return (-b) += *this;
  }

  safe operator * (safe b) const noexcept
  {
    return b *= *this;
  }

  template<class Int2>
  safe operator * (safe<Int2> b) const noexcept
  {
    safe copy(*this);
    return copy *= b;
  }

  template<class Int2>
  safe operator / (safe<Int2> b) const noexcept
  {
    safe<Int> copy(*this);
    return copy /= b;
  }

  safe operator % (safe b) const noexcept
  {
    safe<Int> copy(*this);
    return copy %= b;
  }

  bool operator == (safe b) const noexcept
  {
    return v == b.v;
  }

  bool operator != (safe b) const
  {
    throw_overflow();
    return v != b.v;
  }

  bool operator < (safe b) const
  {
    throw_overflow();
    return v < b.v;
  }

  bool operator <= (safe b) const
  {
    throw_overflow();
    return v <= b.v;
  }

  bool operator > (safe b) const
  {
    throw_overflow();
    return v > b.v;
  }

  bool operator >= (safe b) const
  {
    throw_overflow();
    return v >= b.v;
  }

  explicit operator short() const
  {
    const bool no_ovf2 = 
      v <= std::numeric_limits<short>::max();
    throw_overflow(no_ovf2);
    return (short) v;
  }

  explicit operator unsigned short() const
  {
    const bool no_ovf2 = v >= 0 &&
      v <= std::numeric_limits<unsigned short>::max();
    throw_overflow(no_ovf2);
    return (unsigned short) v;
  }

  explicit operator int() const
  {
    const bool no_ovf2 =
      v <= std::numeric_limits<int>::max();
    throw_overflow(no_ovf2);
    return (int) v;
  }

  explicit operator unsigned int() const
  {
    const bool no_ovf2 = v >= 0 &&
      v <= std::numeric_limits<unsigned int>::max();
    throw_overflow(no_ovf2);
    return (unsigned int) v;
  }

  explicit operator long() const
  {
    const bool no_ovf2 = 
      v <= std::numeric_limits<long>::max();
    throw_overflow(no_ovf2);
    return (long) v;
  }

  explicit operator unsigned long() const
  {
    const bool no_ovf2 = v >= 0 &&
      v <= std::numeric_limits<unsigned long>::max();
    throw_overflow(no_ovf2);
    return (unsigned long) v;
  }

  explicit operator long long() const
  {
    const bool no_ovf2 = 
      v <= std::numeric_limits<long long>::max();
    throw_overflow(no_ovf2);
    return (long long) v;
  }

  explicit operator unsigned long long() const
  {
    const bool no_ovf2 = v >= 0 &&
      v <= std::numeric_limits<unsigned long long>::max();
    throw_overflow(no_ovf2);
    return (unsigned long long) v;
  }

  explicit operator long double () const
  {
    return (Int) *this;
  }

  explicit constexpr operator bool () const
  {
    return no_ovf;
  }

  static constexpr safe overflow() 
  { 
    return safe(); 
  }

  constexpr bool lost_precision() const
  {
    return rem;
  }

  //! Set the lost_precision flag if the argument is true.
  //! \return the rem flag
  bool lost_precision(bool lp) noexcept
  {
    return rem = rem || lp;
  }

protected:
  Int v;

  // TODO what is better?
#if 0
  int8_t 
    no_ovf : 1, 
    rem : 1 = false;
#else
  bool no_ovf;
  //! 
  bool rem = false;
#endif

  constexpr safe(Int val, bool no) noexcept
    : v(val), no_ovf(no) {}

  //! Inherit status bits, typically from the second
  //! operand of an binary function (e.g. operator+)
  template<class Int2>
  void inherit_status(const safe<Int2>& other)
  {
    no_ovf = no_ovf && other.no_ovf;
    rem = rem || other.rem;
  }

  void throw_overflow(bool no_ovf2 = true) const
  {
    if (__builtin_expect(!(no_ovf && no_ovf2), 0))
      throw types::exception<overflow_error>(
        "class safe: unchecked overflow"
      );
  }
};

template<class I>
safe<I> operator + (safe<I> a, I b) noexcept
{
  return a + safe<I>(b);
}

template<class I>
safe<I> operator + (I a, safe<I> b) noexcept
{
  return safe<I>(a) + b;
}

template<class I>
safe<I> operator - (safe<I> a, I b) noexcept
{
  return a - safe<I>(b);
}

template<class I>
safe<I> operator - (I a, safe<I> b) noexcept
{
  return safe<I>(a) - b;
}

template<class I>
safe<I> operator * (safe<I> a, I b) noexcept
{
  return a * safe<I>(b);
}

template<class I>
safe<I> operator * (I a, safe<I> b) noexcept
{
  return safe<I>(a) * b;
}

template<class I>
safe<I> operator / (safe<I> a, I b) noexcept
{
  return a / safe<I>(b);
}

template<class I>
safe<I> operator / (I a, safe<I> b) noexcept
{
  return safe<I>(a) / b;
}

template<class I>
safe<I> operator % (safe<I> a, I b) noexcept
{
  return a % safe<I>(b);
}

template<class I>
safe<I> operator % (I a, safe<I> b) noexcept
{
  return safe<I>(a) % b;
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>&
operator << 
  ( std::basic_ostream<CharT, Traits>& out,
    const safe<Int>& s )
{
  return (bool) s ? out << (Int) s : out << "#overflow";
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>&
operator << 
  ( std::basic_ostream<CharT, Traits>&& out,
    const safe<Int>& s )
{
  return operator <<(out, s);
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_istream<CharT, Traits>&
operator >>
  ( std::basic_istream<CharT, Traits>& in, safe<Int>& s )
{
  Int val;
  typename std::basic_istream<CharT, Traits>::sentry se(in);

  if (!se)
    return in;

  in >> val;
  if (!in.fail())
    s = safe<Int>(val);
  else
    s = safe<Int>::overflow();
  return in;
}

template <
  class Int,
  class CharT,
  class Traits = std::char_traits<CharT>
>
std::basic_istream<CharT, Traits>&
operator >> 
  ( std::basic_istream<CharT, Traits>&& in, safe<Int>& s )
{
  return operator >> (in, s);
}

} // types

#endif
