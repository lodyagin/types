// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * A map which can mix various enumerate<...> as a key.
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
