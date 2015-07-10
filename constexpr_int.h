// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * A wide integer type.
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

#ifndef TYPES_CONSTEXPR_INT_H
#define TYPES_CONSTEXPR_INT_H

#include <cstdint>

namespace types {

namespace constexpr_ {

template<class word_t, size_t n>
struct multiword : multiword<word_t, n-1>
{
  word_t word;
  constexpr multiword() : word(0) {}
  constexpr multiword(word_t i) : word(i) {}
};

template<class word_t>
struct multiword<word_t, 0> 
{
};

} // constexpr_

template<
  size_t bits,
  class word_t = uintmax_t
>
class ulongint
{
public:
  static_assert(
    bits % sizeof(word_t) == 0,
    "types::ulongint: invalid size in bits"
  );

  static constexpr int n_words = bits / sizeof(word_t);

  constexpr ulongint(word_t i) : mw(i) {}

protected:
  constexpr_::multiword<word_t, n_words> mw;
};

} // types

#endif

