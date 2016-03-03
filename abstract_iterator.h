// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
 * Abstract iterator bases for an arbitrary type.
 * An abstract iterator can be used as a virtual function parameter.
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

#ifndef TYPES_ABSTRACT_ITERATOR_H
#define TYPES_ABSTRACT_ITERATOR_H

#include <iterator>
#include <memory>
#include <exception>
#include "types/exception.h"

namespace types {

namespace virtual_iterator {

struct exception : virtual std::exception {};
struct dereference_of_unitialized : exception {};
struct movement_of_unitialized : exception {};
struct incompatible_types : exception {};

//! Abstract constant forward iterator for the type T.
template<class T>
class const_forward_base
{
public:
  using value_type = T;
  using reference = T;
  using pointer = std::unique_ptr<T>;
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;

  virtual ~const_forward_base() {}

  //! @exception dereference_of_unitialized
  virtual reference operator*() const = 0;

  //! @exception movement_of_unitialized
  virtual const_forward_base& operator++() = 0;

  virtual std::unique_ptr<const_forward_base> clone() const
    = 0;

  //! @exception incompatible_types
  virtual bool operator==(const const_forward_base& o) 
    const = 0;

  //! @exception incompatible_types
  bool operator!=(const const_forward_base& o) const 
  { 
    return !operator==(o);
  }

  //! @exception dereference_of_unitialized
  pointer operator->() const
  {
    return std::unique_ptr<T>(new T(operator*()));
  }
};

template<class T>
class const_forward_holder
{
  using base = const_forward_base<T>;

public:
  using value_type = typename base::value_type;
  using reference = typename base::reference;
  using pointer = typename base::pointer;
  using iterator_category = 
    typename base::iterator_category;
  using difference_type = typename base::difference_type;

  const_forward_holder() : ptr(new base()) {}

  const_forward_holder(const base& o) : ptr(o.clone()) {}

  const_forward_holder(const const_forward_holder& o)
    : ptr(o.ptr->clone())
  {}

  const_forward_holder& operator=
    (const const_forward_holder& o)
  {
    ptr = o.clone();
  }

  //! @exception dereference_of_unitialized
  reference operator*() const
  {
    return ptr->operator*();
  }

  //! @exception movement_of_unitialized
  const_forward_holder& operator++()
  {
    ++(*ptr);
    return *this;
  }

  //! @exception incompatible_types
  bool operator==(const const_forward_holder& o) const
  {
    assert(ptr);
    return ptr->operator==(*o.ptr);
  }

  //! @exception incompatible_types
  bool operator!=(const const_forward_holder& o) const 
  { 
    return !operator==(o);
  }

  //! @exception dereference_of_unitialized
  pointer operator->() const
  {
    return ptr->operator->();
  }

protected:
  std::unique_ptr<base> ptr;
};

//! An iterator adapter
template<class It>
class const_forward
  : public const_forward_base<typename It::value_type>
{
  using base = const_forward_base<typename It::value_type>;

public:
  using value_type = typename base::value_type;
  using reference = typename base::reference;
  using pointer = typename base::pointer;
  using iterator_category = 
    typename base::iterator_category;
  using difference_type = typename base::difference_type;

  const_forward(It it_ = It()) : it(it_) {}

  reference operator*() const override
  {
    return *it;
  }

  base& operator++() override
  {
    ++it;
    return *this;
  }

  std::unique_ptr<base> clone() const override
  {
    return std::unique_ptr<base>(new const_forward(it));
  }

  bool operator==(const base& o) const override
  {
    auto* other = dynamic_cast<const const_forward*>(&o);

    if (!other)
      throw types::exception<
        types::virtual_iterator::incompatible_types
      > ();

    return it == other->it;
  }

protected:
  It it;
};

} // virtual_iterator

} // types

#endif

