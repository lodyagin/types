/**
 * @file
 * Type information routines.
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

#ifndef TYPES_TYPEINFO_H
#define TYPES_TYPEINFO_H

#include <typeinfo>
#include <typeindex>
#ifndef _WIN32
#include <cxxabi.h>
#endif

namespace types {

//! Returns a demangled Type name
template<class String>
inline String demangled_name(
  const std::type_index& idx
)
{
#ifndef _WIN32
    // Demangle the name by the ABI rules
    int status;
    const char* mangled = idx.name();
    char* name = abi::__cxa_demangle
      (mangled, nullptr, nullptr, &status);
    if (status == 0) {
      std::string res(name);
      free(name);
      return res;
    }
    else {
      assert(name == nullptr);
      return (String) mangled;
    }
#else
    return (String) typeid(Type).name();
#endif
}

//! Returns a demangled Type name
template<class String>
inline String demangled_name(
  const std::type_info& info
)
{
  return demangled_name<String>(std::type_index(info));
}

template<class String>
String mangled_name(std::type_index code)
{
    return (String) code.name();
}

template<class T>
struct type_of
{
  // unique code for each type
  static std::type_index code() noexcept
  { 
      static std::type_index idx(typeid(T)); 
      return idx;
  }

  //! Returns a demangled Type name
  template<class String>
  static String name()
  {
      return ::types::demangled_name<String>(typeid(T));
  }

  //! For use in context where no dynamic memory
  //! operations are desirable (e.g., throwing an
  //! exception). 
  template<class String>
  static String mangled_name()
  {
      return (String) typeid(T).name();
  }
};

template<class T>
std::string type_name()
{
	return type_of<T>::template name<std::string>();
}

template<class T>
std::string type_name(const T& obj)
{
	return type_name<T>();
}

// selects only a type name, discard a namespace
inline std::string unqualify(const std::string& name)
{
  auto pos = name.find_last_of(':');
  if (pos != std::string::npos)
    return name.substr(pos + 1);
  else
    return name;
}

// use it to get the exact type of T in the error message
template<class T>
struct check_type
{
	using fail = typename std::enable_if<false, T>::type;
};

template<typename... Ts> struct make_void { typedef void type;};
template<typename... Ts> using void_t = typename make_void<Ts...>::type;

} // types

#endif
