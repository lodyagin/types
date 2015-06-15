// -*-coding: mule-utf-8-unix; fill-column: 58; -*-

#ifndef TYPES_SAFE_UNION_H
#define TYPES_SAFE_UNION_H

#include <typeindex>
#include <stdexcept>
#include <utility>

//! A type-safe union for T1 and T2.
//! Can also hold nothing.
//! @author Sergei Lodyagin
template<class T1, class T2>
class safe_union
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
  safe_union() noexcept : the_type(type_t<void>::code()) {}

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
