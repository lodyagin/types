/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Sergei Lodyagin 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute it
  and/or modify it under the terms of the GNU Lesser
  General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU Lesser General Public
  License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 * The implementation of std::put_time (it's absent in
 * GCC) in the Bare C++ style.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#include <ostream>
#include <cstdint>
#include <utility>
#include <chrono>
#include <ratio>
#include <ctime>
#include <iomanip>
#include <tuple>

#ifndef CONCURRO_TYPES_TIME_H_
#define CONCURRO_TYPES_TIME_H_

namespace curr {

namespace time {

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
    using days = duration
      <int, ratio_multiply<hours::period, ratio<24>>>;
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
    std::tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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

} // time

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
  class Duration = typename Clock::duration,
  uint8_t N = 0
>
struct put_time_t
{
  using time_point = std::chrono::time_point<Clock, Duration>;

  put_time_t(const time_point& p, const CharT(&f)[N]) 
    : point(p), format(f) {}

  const time_point point;
  const CharT* format;
};

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration,
  uint8_t N = 0
>
std::ostream& operator<<(
  std::ostream& out, 
  put_time_t<CharT, Clock, Duration, N>&& t
)
{
  const std::tm tmp = time::make_utc_tm(t.point);
  using iterator = std::ostreambuf_iterator<char>;
  using time_put = std::time_put<char, iterator>;
  const time_put& tp= std::use_facet<time_put>(out.getloc());
  const iterator end = tp.put(
    iterator(out.rdbuf()), 
    out, 
    out.fill(), 
    &tmp,
    t.format,
    t.format 
      + std::char_traits<char>::length(t.format)
  );

  if (end.failed())
    out.setstate(std::ios_base::badbit);

  return out;
}

template<
  class CharT, 
  class Clock,
  class Duration = typename Clock::duration,
  uint8_t N = 0
>
put_time_t<CharT, Clock, Duration, N> 
put_time(
  const std::chrono::time_point<Clock, Duration>& time, 
  const CharT(&format)[N]
)
{
  return put_time_t<CharT, Clock, Duration, N>
    (time, format);
}

namespace types {

//! Ostream helper type
template <class Clock, class Duration, class Format>
class put_time_type
{
public:
  typedef std::chrono::time_point<Clock, Duration>
    time_point;

  //! Formatted output. Supports a few conversions from
  //! std::put_time 
  put_time_type(time_point t, const Format& f) 
    : time(t), fmt(f) {}

  put_time_type(time_point t, Format&& f) 
    : time(t), fmt(std::forward<Format>(f)) {}

  time_point time;
  Format fmt;
};

template<class Clock, class Duration>
std::ostream&
operator << 
  (std::ostream& out, 
   const put_time_type<Clock, Duration, const char*>& time
   )
{
#if 0
  const std::tm tm = 
    Clock::time_point_to_tm(time.time);
  out << std::put_time<char>(&tm, time.fmt);
#else
  out << time.fmt;
#endif
  return out;
}

/*
template<class Clock, class Duration, class Format>
std::basic_ostream <
  typename format_string<Format>::char_type,
  std::char_traits<typename format_string<Format>::char_type>
>&
operator << 
  (std::basic_ostream <
    typename format_string<Format>::char_type,
    std::char_traits<typename format_string<Format>::char_type>
   >& out, 
   const put_time_type<Clock, Duration, Format>& time
  );
*/


//! A time stream output helper function (it allows
//! specify a format)
template <class Clock, class Duration, class Format>
put_time_type<Clock, Duration, Format>
//
put_time(std::chrono::time_point<Clock, Duration> time,
         const Format& fmt)
{
  return put_time_type<Clock, Duration, Format>(time, fmt);
}

template <class Clock, class Duration, class Format>
put_time_type<Clock, Duration, Format>
//
put_time(std::chrono::time_point<Clock, Duration> time,
         Format&& fmt)
{
  return put_time_type<Clock, Duration, Format>
    (time, std::forward<Format>(fmt));
}

template <class Clock, class Duration, class CharT>
put_time_type<Clock, Duration, const CharT*>
//
put_time(std::chrono::time_point<Clock, Duration> time,
         const CharT* fmt)
{
  return put_time_type<Clock, Duration, const CharT*>
    (time, fmt);
}

} // types
} // curr

#endif
