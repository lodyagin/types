//-*-coding: mule-utf-8-unix; fill-column: 58; -*-
///////////////////////////////////////////////////////////

/**
 * @file
 * Type information routines.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (C) 2013 Cohors LLC 
 */

#ifndef COHORS_TYPES_TYPEINFO_H
#define COHORS_TYPES_TYPEINFO_H

#include <typeinfo>
#ifndef _WIN32
#include <cxxabi.h>
#endif
#include "types/string.h"

namespace types {

//! Return the demangled Type name
template<class Type, int16_t MaxLen = 32>
struct type
{
  static constexpr int16_t max_len = MaxLen;

  static std::string name()
  {
#ifndef _WIN32
    // Demangle the name by the ABI rules
    int status;
    const char* mangled = typeid(Type).name();
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

  //! For use in context where no dynamic memory
  //! operations are desirable (e.g., throwing an
  //! exception). 
  static auto_string<max_len> mangled_name()
  {
    return auto_string<max_len>(typeid(Type).name());
  }

  operator auto_string<max_len>() const
  {
    return mangled_name();
  }
};

} // types

#endif
