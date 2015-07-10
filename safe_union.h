// -*-coding: mule-utf-8-unix; fill-column: 58; -*-
/**
 * @file
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


#ifndef TYPES_SAFE_UNION_H
#define TYPES_SAFE_UNION_H

#include <typeindex>
#include <stdexcept>
#include <utility>

//! A type-safe union for T...
//! Can also hold nothing.
//! @author Sergei Lodyagin
template<class... T>
class safe_union;

class safe_union_base
{
public:
  template<class T>
  struct type_t
  {
    // unique code for each type
    static std::type_index code() noexcept
    { 
      static std::type_index idx(typeid(type_t<T>)); 
      return idx;
    }
  };
  
  using type_code_t = decltype(type_t<void>::code());

  // try to access for uncontained
  struct type_error : std::logic_error
  {
    type_error(const char* w) : std::logic_error(w) {}
  };

  // void object - both values are uninitialized
  safe_union_base() noexcept : the_type(type_t<void>::code()) {}

protected:
  safe_union_base(type_code_t type_code) : the_type(type_code) {}

  type_code_t the_type;
};

template<class T1, class T2>
class safe_union<T1, T2>
{
public:
  // initialize with T1
  template<class... Args>
  safe_union(type_t<T1> t1, Args&&... args) 
    : t1(std::forward<Args>(args)...),
      the_type(t1.code()) 
  {}

  // initialize with T2
  template<class... Args>
  safe_union(type_t<T2> t2, Args&&... args) 
    : t2(std::forward<Args>(args)...),
      the_type(t2.code()) 
  {}

  // dynamic constructor
  template<class... Args>
  safe_union(type_code_t type_code, Args&&... args)
    : the_type(type_code)
  {
    if (type_code == type_t<T1>::code())
    {
        new(&t1) T1(std::forward<Args>(args)...);
    }
    else if (type_code == type_t<T2>::code())
    {
        new(&t2) T2(std::forward<Args>(args)...);
    }
    else
    {
        throw type_error("the type is not supported by the union");
    }
  }

  safe_union(const safe_union& o)
    : the_type(o.type())
  {
    if (the_type == type_t<T1>::code())
      new(&t1) T1(o.t1);
    else if (the_type == type_t<T2>::code())
      new(&t2) T2(o.t2);
    else if (the_type == type_t<void>::code())
      ; // void
    else
      throw type_error("broken union in copy constructor");
  }

  safe_union(safe_union&& o)
    : the_type(o.type_t())
  {
    if (the_type == type_t<T1>::code())
      new(&t1) T1(std::move(o.t1));
    else if (the_type == type_t<T2>::code())
      new(&t2) T2(std::move(o.t2));
    else if (the_type == type_t<void>::code())
      ; // void
    else
      throw type_error("broken union in move constructor");
  }

  safe_union& operator=(safe_union o)
  {
    swap(o);
    return *this;
  }

  safe_union& operator=(safe_union&& o)
  {
    swap(o);
    return *this;
  }

  ~safe_union()
  {
    if (the_type == type_t<T1>::code())
      t1.~T1();
    else if (the_type == type_t<T2>::code())
      t2.~T2();
    else if (the_type == type_t<void>::code())
      ; // void
    else
      throw type_error("broken union in move constructor");

    the_type = type_t<void>::code();
  }

  void swap(safe_union& o) 
  {
    if (the_type == type_t<T1>::code()) {
      if (o.the_type == type_t<T1>::code())
        // T1 <-> T1
        std::swap(t1, o.t1);
      else if (o.the_type == type_t<T2>::code()) {
        // T1 <-> T2 (1)
        auto t = std::move(o.t2);     // t    <-(t2)-- o
        new(&o.t1) T1(std::move(t1));  // this --(t1)-> o
        new(&t2) T2(std::move(t));    // this <-(t2)-- t
        std::swap(the_type, o.the_type);
      }
      else if (o.the_type == type_t<void>::code()) {
        // T1 <-> void (2)
        new(&o.t1) T1(std::move(t1));  // this --(t1)-> o
        std::swap(the_type, o.the_type);
      }
      else
        throw type_error("broken union in swap (1)");
    }
    else if (the_type == type_t<T2>::code()) {
      if (o.the_type == type_t<T1>::code())
        // T2 <-> T1
        o.swap(*this); // call (1) with swapped args
      else if (o.the_type == type_t<T2>::code())
        // T2 <-> T2
        std::swap(t2, o.t2);
      else if (o.the_type == type_t<void>::code()) {
        // T2 <-> void (3)
        new(&o.t2) T1(std::move(t2));  // this --(t2)-> o
        std::swap(the_type, o.the_type);
      }
      else
        throw type_error("broken union in swap (2)");
    }
    else if (the_type == type_t<void>::code()) {
      if (o.the_type == type_t<T1>::code()
          || o.the_type == type_t<T2>::code())
        // void <-> {T1, T2}
        o.swap(*this); // call (2,3) with swapped args
      else if (o.the_type == type_t<void>::code()) {
        // void <-> void 
        ;
      }
      else
        throw type_error("broken union in swap (2)");
    }
    else
      throw type_error("broken union in swap (3)");
  }

  // a type of current union
  type_code_t type() const { return the_type; }

  // type dictionary
  template<class T>
  static type_code_t code() { return type_t<T>::code(); }

  operator T1&()
  {
    if (the_type == type_t<T1>::code())
      return t1;
    else
      throw type_error("the union doesn't hold T1");
  }

  operator const T1&() const
  {
    return const_cast<safe_union*>(this)->operator T1();
  }

  operator T2&()
  {
    if (the_type == type_t<T2>::code())
      return t2;
    else
      throw type_error("the union doesn't hold T2");
  }

  operator const T2&() const
  {
    return const_cast<safe_union*>(this)->operator T2();
  }

private:
  type_code_t the_type;
  union { T1 t1; T2 t2; };
};


#endif
