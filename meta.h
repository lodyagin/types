/* -*-coding: mule-utf-8-unix; fill-column: 58; -*-

  Copyright (C) 2009, 2013 Cohors LLC 
 
  This file is part of the Cohors Concurro library.

  This library is free software: you can redistribute
  it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be
  useful, but WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General
  Public License along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
*/

/**
 * @file
 * Metaprogramming primitives.
 *
 * @author Sergei Lodyagin
 */

#ifndef CONCURRO_TYPES_META_H_
#define CONCURRO_TYPES_META_H_

#include <type_traits>

namespace curr { namespace types {

//! Check whether flag is true (it is typically a
//! metaprogramming predicate with a value member of a
//! type bool)
//!
//! template<class T>
//! auto fun(...) -> 
//! EnableFunIf(is_same<T, QString>::value, FunReturnType)&
template<bool flag, class RetType>
using EnableFunIf = typename std::remove_reference <
  decltype(typename std::enable_if<flag>::type(),
  std::declval<RetType>()) 
> :: type;

//! This is type expression to check whether `base' is a
//! base of `derived'
#define CURR_ENABLE_BASE_TYPE(base, derived) \
  typename std::enable_if \
    <std::is_base_of<base, derived>::value>::type

//! The quirk to pass a template parameters as an macro
//! argument 
#define CURR_TEMPLATE_AND_PARS(templ, pars...) templ<pars>

}}

#endif
