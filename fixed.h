// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
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

#ifndef TYPES_FIXED_H
#define TYPES_FIXED_H

#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <utility>
#include <type_traits>
#include <ratio>
#include <chrono>
#include <locale>
#include <exception>
#include "types/safe.h"
#include "types/exception.h"

#ifdef QT_GUI
#  include <QMetaObject>
#  include <QString>
#endif

namespace types {

//! Unable represent the number 
//! with the actual fixed_t type.
//! E.g., it can be used in operator"".
struct precision_lost : virtual std::exception {};

template<class Rep, class Ratio>
class fixed_t;

template<
  class CharT, 
  class Rep,
  class Ratio,
  class Traits = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>& 
//
operator<<( 
  std::basic_ostream<CharT, Traits>& out,
  fixed_t<Rep, Ratio> fx
);

template <
  class CharT, 
  class Rep,
  class Ratio,
  class Traits = std::char_traits<CharT>
>
std::basic_istream<CharT>& 
operator>>(
  std::basic_istream<CharT, Traits>& in, 
  fixed_t<Rep, Ratio>& fx
);

/**
 * A fixed point type.
 *
 * @author Anastasia Kurbatova
 * @author Sergei Lodyagin
 */
template<class Rep, class Ratio>
class fixed_t
{
  template<class Re2, class Ra2>
  friend class fixed_t;

  template<class Re, class Ra>
  friend fixed_t operator*(Rep a, fixed_t b) noexcept;

  template<class Int, class Re, class Ra>
  friend fixed_t operator*
    (const types::safe<Int>& a, fixed_t b) noexcept;

  template<class Re, class Ra>
  friend fixed_t operator/(fixed_t a, fixed_t b) noexcept;

  template <class, class, class, class>
  friend class fixed_put;

public:
  typedef Rep modular_type;
  typedef Ratio ratio_type;

  static constexpr auto num = ratio_type::num;
  static constexpr auto den = ratio_type::den;

  static_assert( 
    std::is_signed<modular_type>::value, 
    "modular_type must be signed. "
    "Using unsigned for overflow-controlled arithmetic is "
    "impossible because we don't know what is "
    "(unsigned) -1 really."
  );

  static_assert(
    ratio_type::den != 0,
    "fixed_t::ratio_type::den == 0"
  );

  static_assert(
    ratio_type::num != 0,
    "fixed_t::ratio_type::num == 0"
  );

  static_assert(
    ratio_type::num == 1,
    "fixed_t::ratio_type::num != 1"
  );

  constexpr fixed_t() : rep((Rep)0) {}

  constexpr fixed_t(std::chrono::duration<Rep, Ratio> d)
    : rep(d.count())
  {}

#if 0
#ifdef QT_GUI
  fixed_t(const QVariant& v) 
    // FIXME generic
    : fixed_t(v.toLongLong())
  {
  }
#endif
#endif

  //! The maximal fixed_t value
  static constexpr fixed_t max() 
  {
    return 
      fixed_t(std::numeric_limits<modular_type>::max());
  }

  //! The minimal fixed_t value
  static constexpr fixed_t min() 
  {
    return 
      fixed_t(std::numeric_limits<modular_type>::min());
  }

  static_assert(
    ratio_type::num == 1, 
    "fixed_t::one() is undefined"
  );
  static constexpr fixed_t one()
  {
    return fixed_t(ratio_type::den);
  }

  static constexpr fixed_t zero()
  {
    return fixed_t(0);
  }

  //! Returns the smallest absolute value.
  static constexpr fixed_t bit()
  {
    return fixed_t(1);
  }

  //! Returns an overflow
  static constexpr fixed_t overflow()
  {
    return fixed_t(safe<modular_type>());
  }

  //! Convert to another ratio
  template<class Ra2>
  explicit operator fixed_t<Rep, Ra2>() const
  {
    static_assert(
      Ra2::num == 1, 
      "unsupported fixed_t cast: Ra2::num != 1"
    );
    return fixed_t<Rep, Ra2>(rep);
  }

  //! Convert to long double.
  //! Can loss precision. If lost_precision != nullptr set
  //! *lost_precision = true if either this operation loss
  //! precision or the loss precision flag is already set
  //! in the rep.
  //! \throws std::overflow_exception an overflow in the
  //! past 
  void to_long_double
    (long double& val, bool* lost_precision) const
  {
    val = (long double) rep / den;
    if (lost_precision) // a recursion guard
      *lost_precision = 
        *this != from_long_double(val, nullptr)
        || rep.lost_precision (); 
           // as a result of division
  }

  //! Construct fixed_t from a long double value.
  //! If loss precision set the lost_precision
  //! flag in this fixed_t value and also return in
  //! *loss_precision. If loss_precision == nullptr never
  //! set the flag in the fixed_t value.
  static fixed_t from_long_double
    (long double d, bool* lost_precision) 
  {
    fixed_t p(safe<modular_type>(d * den));
    if (lost_precision) { // a recursion guard
      long double d2;
      p.to_long_double(d2, nullptr);
      *lost_precision = p.rep.lost_precision(d != d2);
    }
    return p;
  }

  void clear_precision_lost()
  {
    rep = safe<modular_type>((Rep) rep);
  }

  //! Return an absolute value
  fixed_t abs() const noexcept
  {
    return fixed_t(rep.abs());
  }

  fixed_t operator - () const noexcept
  { 
    return fixed_t(-rep); 
  }

  bool operator < (fixed_t p) const
  {
    return rep < p.rep;
  }

  bool operator > (fixed_t p) const noexcept
  {
    return rep > p.rep;
  }
  bool operator <= (fixed_t p) const noexcept
  {
    return !operator>(p);
  }

  bool operator >= (fixed_t p) const noexcept
  {
    return !operator<(p);
  }

  bool operator == (fixed_t p) const noexcept
  {
   return rep == p.rep;
  }

  bool operator!=(fixed_t p) const noexcept
  {
    return !operator==(p);
  }

  fixed_t& operator += (fixed_t p) noexcept
  {
    rep += p.rep;
    return *this;
  }

  fixed_t& operator -= (fixed_t p) noexcept
  {
    rep -= p.rep;
    return *this;
  }

  fixed_t& operator *= (modular_type p) noexcept
  {
    rep *= p;
    return *this;
  }

  template<class Int>
  fixed_t& operator *= 
    (const types::safe<Int>& p) noexcept
  {
    rep *= p;
    return *this;
  }

  template<class Ra2>
  fixed_t& operator *= (fixed_t<Rep, Ra2> p) noexcept
  {
    const safe<Rep> copy = rep;
    rep *= p.rep;
    rep *= Ra2::num;
    rep /= Ra2::den;
    if (!rep /*&& (bool) copy*/) {
      rep = copy;
      rep /= Ra2::den;
      rep *= Ra2::num;
      rep *= p.rep;
    }
    return *this;
  }

  fixed_t& operator /= (modular_type p) noexcept
  {
    rep /= p;
    return *this;
  }

  template<class Int>
  fixed_t& operator /=
    (const types::safe<Int>& p) noexcept
  {
    rep /= p;
    return *this;
  }

  template<class Ra2>
  fixed_t& operator/=(fixed_t<Rep, Ra2> o) noexcept
  {
    const safe<Rep> copy = rep;
    rep *= Ra2::den;
    rep /= Ra2::num;
    rep /= o.rep;
    if (!rep) {
      rep = copy;
      rep /= o.rep;
      rep /= Ra2::num;
      rep *= Ra2::den;
    }
     return *this;
  }

  fixed_t operator + (fixed_t p) const noexcept
  {
    return fixed_t(rep + p.rep);
  }

  fixed_t operator - (fixed_t p) const noexcept
  {
    return fixed_t(rep - p.rep);
  }
  
  fixed_t operator * (modular_type p) const noexcept
  {
    return fixed_t(rep * p);
  }

  template<class Int>
  fixed_t operator * (const types::safe<Int>& p) const 
    noexcept
  {
    return fixed_t(rep * p);
  }

  template<class Rep2, class Rat2>
  fixed_t operator * (fixed_t<Rep2, Rat2> p) const noexcept
  {
    fixed_t res(*this);
    res *= p;
    return res;
  }

  //! Divizion with truncation.
  //! Division by zero is stored as an overflow
  fixed_t operator / (modular_type p) const noexcept
  {
    return fixed_t(rep / p);
  }

  //! Divizion with truncation.
  //! Division by zero is stored as an overflow
  template<class Int>
  fixed_t operator / 
    (const types::safe<Int>& p) const noexcept
  {
    return fixed_t(rep / p);
  }

  fixed_t operator % (fixed_t b) const noexcept
  {
    // Alex Stepanov's explains the proper sign
    // of % operation, C++-11 do according to it.
    return fixed_t(rep % b.rep);
  }

  //! Check an overflow occurence in the past
  explicit operator bool() const noexcept
  {
    return (bool) rep;
  }

  //! Truncates to an integer
  safe<modular_type> truncate() const noexcept
  {
    safe<modular_type> copy(rep);
    copy *= ratio_type::num;
    copy /= ratio_type::den;
    return copy;
  }

#ifdef QT_GUI
  operator QGenericArgument() const
  {
    return Q_ARG(fixed_t, *this);
  }

#if 0
  explicit operator QVariant() const
  {
    return QVariant((qint64) (Rep) rep);
  }
#endif

  QString toQString() const
  {
    std::ostringstream str;
    str << *this;
    return QString(str.str().c_str());
  }
#endif

protected:
  safe<modular_type> rep;

  // TODO make protected
  explicit constexpr fixed_t
    (const types::safe<modular_type>& arep) 
    : rep(arep) {}  

  explicit constexpr fixed_t(modular_type arep)
    : fixed_t(types::safe<modular_type>(arep)) {}
};

template<class Re, class Ra>
fixed_t<Re, Ra> operator*(
  typename fixed_t<Re, Ra>::modular_type a, 
  fixed_t<Re, Ra> b
) noexcept
{
  return b.operator*(a);
}

template<class Int, class Re, class Ra>
fixed_t<Re, Ra> operator*( 
  const types::safe<Int>& a, 
  fixed_t<Re, Ra> b 
) noexcept
{
  return b.operator*(a);
}

template<class Re, class Ra>
fixed_t<Re,Ra> operator/(
  fixed_t<Re,Ra> a, 
  fixed_t<Re,Ra> b
) noexcept
{
  return a /= b;
}

template<class Rep, class Ratio>
constexpr fixed_t<Rep, Ratio> 
//
to_fixed(std::chrono::duration<Rep, Ratio> dur)
{
  return fixed_t<Rep, Ratio>(dur);
}

template <
  class CharT,
  class Rep,
  class Ratio,
  class OutputIt = std::ostreambuf_iterator<CharT>
>
class fixed_put : public std::locale::facet
{
public:
  typedef CharT char_type;
  typedef std::ostreambuf_iterator<CharT> iter_type;
  
  static std::locale::id id;

  explicit fixed_put(size_t refs = 0) 
    : std::locale::facet(refs)
  {}

  iter_type put
   ( iter_type out, 
      std::ios_base& str, 
      char_type fill, 
      fixed_t<Rep, Ratio> v ) const
  {
    return do_put(out, str, fill, v);
  }
  
protected:
  ~fixed_put() {}

  virtual iter_type do_put( 
    iter_type out, 
    std::ios_base& str, 
    char_type fill, 
    fixed_t<Rep, Ratio> v 
  ) const
  {
    using namespace ::types;
    typedef std::basic_string<CharT> string_type;
    typedef fixed_t<Rep, Ratio> fixed_t;

    const auto& numpunct = 
      std::use_facet<std::numpunct<CharT>>(str.getloc());
    const auto& numput = 
     std::use_facet<std::num_put<CharT, iter_type>>
       (str.getloc());

    if (! (bool) v.rep ) {
      const string_type ovf = numpunct.falsename();
      std::copy(ovf.begin(), ovf.end(), out);
      return out;
    }

    int frac = -1;

    // count the number of frac digits needed to represent
    // without any loss

    if (frac < 0) {

      fixed_t p2(v % fixed_t::one()); // fractional part only
      frac = 0;

      // checking overflow in p2 preventing from a possible
      // infinite loop
      while(p2 % fixed_t::one() != fixed_t::zero() 
            && (bool) p2
            )
      {
        ++frac; p2 *= 10;
      }

      if (! (bool) p2) {
        // the error mark (four dots)
        *out++ = numpunct.decimal_point();
        *out++ = numpunct.decimal_point();
        *out++ = numpunct.decimal_point(); 
        *out++ = numpunct.decimal_point(); 
        return out;
      }
    }
  
    const auto old_flags = str.flags();
    str.setf(std::ios_base::fixed);
    str.precision(frac);

    long double ld;
    bool lost_precision;
    v.to_long_double(ld, &lost_precision);

    // here it is!
    numput.put(out, str, fill, ld);
    str.setf(old_flags);

    if (lost_precision) {
      // the error mark (two dots)
      *out++ = numpunct.decimal_point();
      *out++ = numpunct.decimal_point();
    }
    return out;
  }
};

template <
  class CharT,
  class Rep,
  class Ratio,
  class OutputIt
>
std::locale::id fixed_put<CharT, Rep, Ratio, OutputIt>
::id;

template <
  class CharT,
  class Rep,
  class Ratio,
  class InputIt = std::istreambuf_iterator<CharT>
>
class fixed_get : public std::locale::facet
{
public:
  typedef CharT char_type;
  typedef std::istreambuf_iterator<CharT> iter_type;
  
  static std::locale::id id;

  explicit fixed_get(size_t refs = 0) 
    : std::locale::facet(refs)
  {}

  iter_type get(
    iter_type in, 
    iter_type end,
    std::ios_base& str, 
    std::ios_base::iostate& err, 
    fixed_t<Rep, Ratio>& v 
  ) const
  {
    return do_get(in, end, str, err, v);
  }
  
protected:
  ~fixed_get() {}

  virtual iter_type do_get( 
    iter_type in, 
    iter_type end,
    std::ios_base& str, 
    std::ios_base::iostate& err, 
    fixed_t<Rep, Ratio>& v 
  ) const
  {
    long double ld;
    const auto& numget =
     std::use_facet<std::num_get<CharT, iter_type>>
       (str.getloc());
    const auto it = numget.get(in, end, str, err, ld);

    if (err & std::ios_base::failbit)
      return it;

    bool lost_precision; // it is also set in v
    v = fixed_t<Rep, Ratio>::from_long_double(
      ld, 
      &lost_precision 
    );
    return it;
  }
};

template <
  class CharT,
  class Rep,
  class Ratio,
  class InputIt
>
std::locale::id fixed_get<CharT, Rep, Ratio, InputIt>
::id;

template<
  class CharT, 
  class Rep,
  class Ratio,
  class Traits// = std::char_traits<CharT>
>
std::basic_ostream<CharT, Traits>& 
//
operator<<( 
  std::basic_ostream<CharT, Traits>& out,
  fixed_t<Rep, Ratio> fx
)
{
  using namespace std;
  using iterator = ostreambuf_iterator<CharT, Traits>;
  using fixed_put = fixed_put<CharT, Rep, Ratio, iterator>;

  try {
    typename basic_ostream<CharT, Traits>::sentry s(out);
    if (s) {
      std::locale locale(out.getloc(), new fixed_put);
      const fixed_put& fp = use_facet<fixed_put>(locale);

      const iterator end = fp.put(
        iterator(out.rdbuf()), 
        out, 
        out.fill(), 
        fx
      );
      if (end.failed())
        out.setstate(ios_base::badbit);
    }
  }
  catch (...) {
    out.setstate(ios_base::badbit);
  }
  return out;
}

template <
  class CharT, 
  class Rep,
  class Ratio,
  class Traits// = std::char_traits<CharT>
>
std::basic_istream<CharT>& 
operator>>(
  std::basic_istream<CharT, Traits>& in, 
  fixed_t<Rep, Ratio>& fx
)
{
  using namespace std;
  using iterator = istreambuf_iterator<CharT, Traits>;
  using fixed_get = fixed_get<CharT, Rep, Ratio, iterator>;

  try {
    typename basic_istream<CharT, Traits>::sentry s(in);
    if (s) {
      std::locale locale(in.getloc(), new fixed_get);
      const fixed_get& fg = use_facet<fixed_get>(locale);
      ios_base::iostate error = ios_base::goodbit;
      const iterator end;
  
      const iterator it = fg.get(
        iterator(in), 
        end,
        in, 
        error,
        fx
      );
      if (it.equal(end))
        in.setstate(ios_base::eofbit);
    }
  }
  catch (...) {
    in.setstate(ios_base::badbit);
  }
  return in;
}

} // types

#endif
