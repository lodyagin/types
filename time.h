// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#include <cstdint>
#include <ostream>
#include <utility>
#include <chrono>
#include <ratio>
#include <ctime>
#include <iomanip>
//#include "conversion.h"

#ifndef CONCURRO_TYPES_TIME_H_
#define CONCURRO_TYPES_TIME_H_

namespace curr {

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
