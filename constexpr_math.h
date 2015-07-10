// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * A constexpr arithmetic.
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

#ifndef TYPES_CONSTEXPR_MATH_H
#define TYPES_CONSTEXPR_MATH_H

#include <cstdint>

namespace types {

// FIXME add overflow
template<uintmax_t x>
struct log2x 
{
  enum : uintmax_t{ value = log2x<(x >> 1)>::value + 1 };
};

template<> 
struct log2x<1>
{
  enum { value = 0 };
};

// FIXME add overflow
// value is number of radix digits in x
template<unsigned radix, uintmax_t x>
struct digits
{
  enum : uintmax_t{ value = digits<radix, x / radix>::value + 1 };
};

template<unsigned radix> 
struct digits<radix, 0> { enum { value = 0}; };

template<uint8_t x>
struct pow2x
{
  enum : uintmax_t { value = 2 * pow2x<x - 1>::value };
};

template<>
struct pow2x<0>
{
  enum : uintmax_t { value = 1 };
};

template<uint8_t x>
struct pow10x
{
  enum : uintmax_t { value = 10 * pow10x<x - 1>::value };
};

template<>
struct pow10x<0>
{
  enum : uintmax_t { value = 1 };
};

//! Return mask with n lower bits set
template<uint8_t n, class T = uint8_t>
constexpr T n_bits_mask()
{
  return pow2x<n+1>::value - 1;
}

} // types

#endif



