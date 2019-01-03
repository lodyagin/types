// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * "case" operator version for code with variardic templates.
 *
 * This file (originally) was a part of public
 * https://github.com/lodyagin/types repository.
 *
 * @author Sergei Lodyagin
 * @copyright Copyright (c) 2015, Sergei Lodyagin
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

#ifndef TYPES_TEMPLATED_SWITCH_H
#define TYPES_TEMPLATED_SWITCH_H

#include <tuple>
#include <utility>
#include "types/meta.h"

namespace types
{

// It calls function based on a key, like an ordinary C language
// switch operator does. It returns false if no case for this key
// provided (C switch "default" action).
// It is implemented as a recursion.

template<class Selector, class Pars>
bool do_switch(const Selector&, Pars&&)
{
    return false;
}

// TODO hashtable implementation (test performance first)
template<class Selector, class Pars, class Case, class... Cases>
bool do_switch(
    const Selector& key, 
    Pars&& pars, 
    Case case0, 
    Cases... cases
)
{
    return tuple::call(case0, std::forward<Pars>(pars), key)
        || do_switch(key, std::forward<Pars>(pars), cases...);
}

template<
  template<class> class CaseT,
  class Selector, 
  class Pars, 
  class... CasePars
>
bool do_switch_templ_cases_(const Selector& key, Pars&& pars)
{
  return do_switch(
    key, 
    std::forward<Pars>(pars), 
    CaseT<CasePars>()...
  );
}

template<
  template<class> class CaseT,
  class Selector, 
  class Pars, 
  class... CasePars
>
bool do_switch_tuple(
    const Selector& key, 
    Pars&& pars, 
    const std::tuple<CasePars...>&
)
{
  return do_switch_templ_cases_<CaseT, Selector, Pars, CasePars...>(
    key, 
    std::forward<Pars>(pars)
  );
}

template<class Selector, class Case, class Fun, class... Pars>
bool switch_case(
    const Selector& key, 
    const Case& case_, 
    Fun fun, 
    Pars&&... pars
)
{
    return (key == case_) 
        ? (fun(std::forward<Pars>(pars)...), true) 
        : false;
}

}

#endif
