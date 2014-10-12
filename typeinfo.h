//-*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file
 * Type information routines.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef TYPES_TYPEINFO_H
#define TYPES_TYPEINFO_H

#include <typeinfo>
#include <typeindex>
#ifndef _WIN32
#include <cxxabi.h>
#endif
#include "types/string.h"

namespace types {

//! Returns a demangled Type name
inline std::string demangled_name(
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
      return std::string(mangled);
    }
#else
    return typeid(Type).name();
#endif
}

//! Returns a demangled Type name
inline std::string demangled_name(
  const std::type_info& info
)
{
  return demangled_name(std::type_index(info));
}

#if 0
template<int16_t MaxLen>
inline auto_string<MaxLen> mangled_name(
  const std::type_index& idx
)
{
   const std::string name = idx.name();
   return auto_string<MaxLen>(
     // get the last (most informative) part of the name
     std::max(name.begin(), name.end() - name.size()),
     name.end()
   );
}

template<int16_t MaxLen>
inline auto_string<MaxLen> mangled_name(
  const std::type_info& info
)
{
  return mangled_name<MaxLen>(std::type_index(info));
}
#endif

template<class Type/*, int16_t MaxLen = 40*/>
struct type
{
//  static constexpr int16_t max_len = MaxLen;

  //! Returns a demangled Type name
  static std::string name()
  {
    return ::types::demangled_name(typeid(Type));
  }

  //! For use in context where no dynamic memory
  //! operations are desirable (e.g., throwing an
  //! exception). 
  static const char* mangled_name()
  {
    return typeid(Type).name();
  }

#if 0
  operator auto_string<max_len>() const
  {
    return mangled_name();
  }
#endif
};

} // types

#endif
