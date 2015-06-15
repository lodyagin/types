// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * A map which can mix various enumerate<...> as a key.
 *
 * @author Sergei Lodyagin
 */


#ifndef TYPES_ENUM_MAP_H
#define TYPES_ENUM_MAP_H

//#include <type_traits>
#include "types/enum.h"

namespace types {

template<
  class T,
  template<class...> class Map,
  class... MapArgs
>
class enum_map 
  : public Map<std::type_index, T, MapArgs...>
{
  // NB no data members
  using map = Map<std::type_index, T, MapArgs...>;

public:
  //template<class... EnumVal>
  //struct selector_t;

  template<class Enum>
  T& operator[](Enum e)
  {
    return map::operator[](enum_type_index<Enum>(e));
  }

  //! Merge sets
  enum_map& operator|=(const enum_map& o)
  {
    this->insert(o.begin(), o.end());
    return *this;
  }

  /*template<class... EnumVal>
  selector_t<EnumVal...>&&  // protect storing?
  select()
  {
    return selector_t<EnumVal...>(*this);
  }*/
};

} // types

#endif
