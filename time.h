// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * The implementation of std::put_time (it's absent in
 * GCC) in the Bare C++ style.
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

#include <ostream>
#include <cstdint>
#include <utility>
#include <chrono>
#include <ratio>
#include <ctime>
#include <iomanip>
#include <tuple>
#include "types/string.h"

#ifndef COHORS_TYPES_TIME_H_
#define COHORS_TYPES_TIME_H_

namespace times {

namespace howard_hinnant {

//! Returns number of days since civil 1970-01-01.  Negative values indicate
//!    days prior to 1970-01-01.
//! Preconditions:  y-m-d represents a date in the civil (Gregorian) calendar
//!                 m is in [1, 12]
//!                 d is in [1, last_day_of_month(y, m)]
//!                 y is "approximately" in
//!                   [numeric_limits<Int>::min()/366, numeric_limits<Int>::max()/366]
//!                 Exact range of validity is:
//!                 [civil_from_days(numeric_limits<Int>::min()),
//!                  civil_from_days(numeric_limits<Int>::max()-719468)]
//! Howard Hinnant
//! chrono-Compatible Low-Level Date Algorithms
//! http://home.roadrunner.com/~hinnant/date_algorithms.html
template <class Int>
// constexpr c++14
Int
days_from_civil(Int y, unsigned m, unsigned d) noexcept
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    y -= m <= 2;
    const Int era = (y >= 0 ? y : y-399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);      // [0, 399]
    const unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d-1;  // [0, 365]
    const unsigned doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]
    return era * 146097 + static_cast<Int>(doe) - 719468;
}

//! Returns year/month/day triple in civil calendar
//! Preconditions:  z is number of days since 1970-01-01 and is in the range:
//!                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-719468].
//! Howard Hinnant
//! chrono-Compatible Low-Level Date Algorithms
//! http://home.roadrunner.com/~hinnant/date_algorithms.html
template <class Int>
//constexpr (c++14)
std::tuple<Int, unsigned, unsigned> 
civil_from_days(Int z) noexcept
{
    static_assert(std::numeric_limits<unsigned>::digits >= 18,
             "This algorithm has not been ported to a 16 bit unsigned integer");
    static_assert(std::numeric_limits<Int>::digits >= 20,
             "This algorithm has not been ported to a 16 bit signed integer");
    z += 719468;
    const Int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
    const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
    const Int y = static_cast<Int>(yoe) + era * 400;
    const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
    const unsigned mp = (5*doy + 2)/153;                                   // [0, 11]
    const unsigned d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
    const unsigned m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
    return std::tuple<Int, unsigned, unsigned>(y + (m <= 2), m, d);
}

//! Returns day of week in civil calendar [0, 6] -> [Sun, Sat]
//! Preconditions:  z is number of days since 1970-01-01 and is in the range:
//!                   [numeric_limits<Int>::min(), numeric_limits<Int>::max()-4].
//! Howard Hinnant
//! chrono-Compatible Low-Level Date Algorithms
//! http://home.roadrunner.com/~hinnant/date_algorithms.html
template <class Int>
// constexpr c++14
unsigned weekday_from_days(Int z) noexcept
{
    return static_cast<unsigned>(z >= -4 ? (z+4) % 7 : (z+5) % 7 + 6);
}

//! Howard Hinnant
//! chrono-Compatible Low-Level Date Algorithms
//! http://home.roadrunner.com/~hinnant/date_algorithms.html
template <class To, class Rep, class Period>
To round_down(const std::chrono::duration<Rep, Period>& d)
{
    To t = std::chrono::duration_cast<To>(d);
    if (t > d)
        --t;
    return t;
}

} // howard_hinnant

//! Converts std::chrono::time_point<system_clock> to
//! std::tm in UTC. Great thanks to Howard Hinnant,
//! http://stackoverflow.com/a/16784256/1326885
//! FIXME: leap seconds 
//! (convert by the table at
//! http://en.wikipedia.org/wiki/Leap_seconds)
template <class Duration>
std::tm make_utc_tm(
  std::chrono::time_point
    <std::chrono::system_clock, Duration> tp
)
{
    using namespace std;
    using namespace std::chrono;
    using days = duration<
      int, 
      typename std::ratio_multiply
        <hours::period, std::ratio<24>>::type
    >;
    // t is time duration since 1970-01-01
    Duration t = tp.time_since_epoch();
    // d is days since 1970-01-01
    days d = howard_hinnant::round_down<days>(t);
    // t is now time duration since midnight of day d
    t -= d;
    // break d down into year/month/day
    int year;
    unsigned month;
    unsigned day;
    std::tie(year, month, day) = 
      howard_hinnant::civil_from_days(d.count());
    // start filling in the tm with calendar info
#ifdef _WIN32
    std::tm tm = {0};
#else
    std::tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_wday = howard_hinnant::weekday_from_days(d.count());
    tm.tm_yday = d.count() - howard_hinnant::days_from_civil(year, 1, 1);
    // Fill in the time
    tm.tm_hour = duration_cast<hours>(t).count();
    t -= hours(tm.tm_hour);
    tm.tm_min = duration_cast<minutes>(t).count();
    t -= minutes(tm.tm_min);
    tm.tm_sec = duration_cast<seconds>(t).count();
    return tm;
}

//! Log the current time when output to a stream
template<class CharT, class Clock, size_t slen>
class timestamp_t
{
public:
  timestamp_t(const CharT(&fmt)[slen])
    : format(fmt)
  {}

  const strings::basic_constexpr_string<CharT> format;
};

template<class Clock, size_t slen>
timestamp_t<char, Clock, slen> timestamp(
  const char(&s)[slen]
)
{
  return timestamp_t<char, Clock, slen>(s);
}

template<class Clock, size_t slen>
timestamp_t<wchar_t, Clock, slen> timestamp(
  const wchar_t(&s)[slen]
)
{
  return timestamp_t<wchar_t, Clock, slen>(s);
}

//! seconds from the last midnight
template < 
  class Clock,
  class SDur,
  class TDur = std::chrono::duration<typename SDur::rep> 
  // 1 sec per
>
TDur seconds_since_midnight
  (std::chrono::time_point<Clock, SDur> time_point)
{
  return std::chrono::duration_cast<TDur>
      (time_point.time_since_epoch())
        % std::chrono::hours(24);
}

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration//,
//  uint8_t N = 0
>
struct put_time_t
{
  using time_point = std::chrono::time_point<Clock, Duration>;

  template<size_t N>
  put_time_t(const time_point& p, const CharT(&f)[N]) 
    : point(p), format(f) 
  {}

  put_time_t(
    const time_point& p, 
    strings::constexpr_string f
  ) 
    : point(p), format(f.c_str()) 
  {}

  const time_point point;
  const CharT* format;
};

} // times

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration,
  uint8_t N = 0
>
::times::put_time_t<CharT, Clock, Duration> 
put_time(
  const std::chrono::time_point<Clock, Duration>& time, 
  const CharT(&format)[N]
)
{
  return ::times::put_time_t<CharT, Clock, Duration>
    (time, format);
}

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration
>
::times::put_time_t<CharT, Clock, Duration> 
put_time(
  const std::chrono::time_point<Clock, Duration>& time, 
  const strings::basic_constexpr_string<CharT> format
)
{
  return ::times::put_time_t<CharT, Clock, Duration>
    (time, format);
}

namespace times {

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration,
  uint8_t N = 0
>
void put_time(
  std::basic_ios<CharT>& out, 
  const put_time_t<CharT, Clock, Duration>& t
)
{
  const std::tm tmp = ::times::make_utc_tm(t.point);
  using iterator = std::ostreambuf_iterator<CharT>;
  using time_put = std::time_put<CharT, iterator>;
  const time_put& tp= std::use_facet<time_put>(out.getloc());
  const iterator end = tp.put(
    iterator(out.rdbuf()), 
    out, 
    out.fill(), 
    &tmp,
    t.format,
    t.format 
      + std::char_traits<CharT>::length(t.format)
  );

  if (end.failed())
    out.setstate(std::ios_base::badbit);
}

template<
  class CharT, 
  class Clock, 
  size_t slen
>
void put_time(
  std::basic_ios<CharT>& out, 
  const ::times::timestamp_t<CharT, Clock, slen>& ts
)
{
  put_time(out, ::put_time(Clock::now(), ts.format));
}

} // times

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration,
  uint8_t N = 0
>
std::basic_ostream<CharT>& operator<<(
  std::basic_ostream<CharT>& out, 
  const ::times::put_time_t<CharT, Clock, Duration>& t
)
{
  ::times::put_time(out, t);
  return out;
}

template<
  class CharT, 
  class Clock, 
  size_t slen
>
std::basic_ostream<CharT>& operator<<(
  std::basic_ostream<CharT>& out, 
  const ::times::timestamp_t<CharT, Clock, slen>& ts
)
{
  ::times::put_time(out, ts);
  return out;
}

#endif
