// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * An array with types::enumerate as an index.
 *
 * @author Sergei Lodyagin
 */


#ifndef TYPES_ENUM_ARRAY_H
#define TYPES_ENUM_ARRAY_H

#include <array>
#include <type_traits>
#include "types/enum.h"

namespace types {

template<class Enum, class T>
class enum_array : protected std::array<T, Enum::size()>
{
  using array = std::array<T, Enum::size()>;
  //NB no data members
public:
  using index_type = Enum;
  using value_type = T;
  using size_type = typename 
    std::make_unsigned<Enum::int_type>::type;

  enum_array() {}

  T& operator[](Enum key)
  {
    return array::operator[key.index()];
  }

  static constexpr size()
  {
    return Enum::size();
  }
};

} // types

#endif
