//-*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_EXCEPTION_H
#define COHORS_TYPES_EXCEPTION_H

#include <iostream>
#include <tuple>
#include <algorithm>
#include <iterator>
#include "types/string.h"
#include "types/compound_message.h"

namespace types {

//! An std::exception descendant wich holds a string
//! buffer. It relaxes dynamic storage requirements in the
//! time of throwing the exception.  
//!
//! Itanium C++ ABI: Exception Handling 
//! 3.3.1 Exception Storage
//!
//! The C++ runtime library shall allocate a static
//! emergency buffer of at least 4K bytes per potential
//! task, up to 64KB. This buffer shall be used only in
//! cases where dynamic allocation of exception objects
//! fails. It shall be allocated in 1KB chunks. At most 16
//! tasks may use the emergency buffer at any time, for at
//! most 4 nested exceptions, with each exception object
//! (including header) of size at most 1KB. Additional
//! threads are blocked until one of the 16 de-allocates
//! its emergency buffer storage.  
//!
//! @tparam the size of the string buffer 
//!         (including ending 0) 
//! @tparam the maximal size of the string lower than the
//!         the minimal size of an emergency biffer
template<
  uint16_t max_len,
  uint16_t emergency_string_limit = 512
  //< If I had more time, I would have written 
  //< a shorter letter. M. Twain
>
class exception_string : public virtual std::exception
{
public:
  static constexpr uint16_t buf_len = 
    types::min(max_len, emergency_string_limit);

  using stringbuf = auto_stringbuf<buf_len>;
  using string = typename stringbuf::string;

  exception_string() 
  {
    std::fill(msg.str().begin(), msg.str().buf_end(), 0);
  }

  explicit exception_string(const char(&message)[max_len]) 
    : exception_string()
  {
    std::copy( 
      message, 
      message + buf_len, // limit the message size
      msg.str().begin()
    );
  }

  exception_string(const exception_string& o)
    : exception_string()
  {
    msg.str(o.msg.str());
  }

  exception_string(
    typename string::const_iterator begin,
    typename string::const_iterator end
  )
    : exception_string()
  {
    std::copy(begin, end, msg.str().begin());
  }

  exception_string& operator=(const exception_string& o)
  {
    std::fill(msg.str().begin(), msg.str().buf_end(), 0);
    msg.str(o.msg.str());
    return *this;
  }

  const char* what() const noexcept override
  {
    // msg.str() is a cycled buffer, if it overrun it can
    // contain '\0' in the middle. We need clean it out to
    // not lost the exception information.
    // TODO better start from the end and go while nonzero
    // found, then replace 0->? starting from this place
    // to the begin (it will keep rubbish if any for
    // analize). 
    std::replace( 
      msg.str().begin(), 
      std::min(msg.str().end(), msg.str().buf_end()), 
      '\0',
      '?'
    ); 
    return msg.str().c_str();
  }

protected:
  mutable stringbuf msg;
};

template<class... Pars>
class exception_compound_message 
  : public exception_string<
      compound_message_max_length<Pars...>() + 1
    >
{
  using parent = exception_string<
    compound_message_max_length<Pars...>() + 1
  >;

public:
  const compound_message_t<
    std::ostreambuf_iterator<char>,
    Pars...
  > message;

  explicit exception_compound_message(Pars&&... pars)
    : parent(), 
      message(std::forward<Pars>(pars)...)
  {
    auto it = std::ostreambuf_iterator<char>(&this->msg);
    message.stringify(
      it,
      std::cout //exception_::the_ostream
    );
    //*it = '\0';
    // the stream iterator does not include ending 0,
    // it is the part of the underlaying auto_string
  }
};

namespace formatted_ {

template<class Exception, class... Args>
struct exception
  : Exception, 
    exception_compound_message<Args...> 
{
  exception(Args&&... args) 
    : exception_compound_message<Args&&...>
        (std::forward<Args>(args)...) {}

  exception(const Exception& exc, Args&&... args) 
    : Exception(exc),
      exception_compound_message<Args&&...>
        (std::forward<Args>(args)...) {}

/*  exception(Exception&& exc, Args&&... args) 
    : Exception(std::forward<Exception>(exc)),
      exception_compound_message<Args&&...>
        (std::forward<Args>(args)...) {}*/
};

}

template<class Exception, class... Args>
auto exception(Args&&... args)
  -> formatted_::exception<Exception, Args&&...>
{
  return formatted_::exception<Exception, Args&&...>
    (std::forward<Args>(args)...);
}

template<class Exception, class... Args>
auto exception(Exception&& exc, Args&&... args)
  -> formatted_::exception<
       typename std::decay<Exception>::type, Args&&...>
{
  return formatted_::exception<
    typename std::decay<Exception>::type, Args&&...>(
    std::forward<Exception>(exc),
    std::forward<Args>(args)...
  );
}

} // types

#endif


