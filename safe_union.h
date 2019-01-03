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

#include <functional>
#include <typeindex>
#include <stdexcept>
#include <utility>
#include "types/templated_switch.h"
#include "types/traits.h"
#include "types/exception.h"
#include "types/typeinfo.h"
#include "types/strings/mixed.h"

namespace types
{

//! A type-safe union for T...
//! Can also hold nothing.
//! @author Sergei Lodyagin

template<class... T>
class safe_union;


using type_code_t = decltype(type_of<void>::code());

// We don't use std::logic_error because
// it has no const char* constructor in GNU library.
struct logic_error : virtual std::exception {};

struct type_error : types::logic_error {};


namespace union_
{

template<class... T>
struct holder;

template<class T1, class T2>
struct holder<T1, T2>
{ 
    union { T1 t1; T2 t2; }; 

    holder() noexcept {}

    template<class... Args>
    holder(type_of<T1>, Args&&... args) 
        : t1(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T2>, Args&&... args) 
        : t2(std::forward<Args>(args)...)
    {}

    ~holder() {}

    operator T1&() { return t1; }
    operator T2&() { return t2; }
};

template<class T1, class T2, class T3>
struct holder<T1, T2, T3>
{ 
    union { T1 t1; T2 t2; T3 t3; }; 

    holder() noexcept {}

    template<class... Args>
    holder(type_of<T1>, Args&&... args) 
        : t1(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T2>, Args&&... args) 
        : t2(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T3>, Args&&... args) 
        : t3(std::forward<Args>(args)...)
    {}

    ~holder() {}

    operator T1&() { return t1; }
    operator T2&() { return t2; }
    operator T3&() { return t3; }
};

template<class T1, class T2, class T3, class T4>
struct holder<T1, T2, T3, T4>
{ 
    union { T1 t1; T2 t2; T3 t3; T4 t4; }; 

    holder() noexcept {}

    template<class... Args>
    holder(type_of<T1>, Args&&... args) 
        : t1(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T2>, Args&&... args) 
        : t2(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T3>, Args&&... args) 
        : t3(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T4>, Args&&... args) 
        : t4(std::forward<Args>(args)...)
    {}

    ~holder() {}

    operator T1&() { return t1; }
    operator T2&() { return t2; }
    operator T3&() { return t3; }
    operator T4&() { return t4; }
};

template<class T1, class T2, class T3, class T4, class T5>
struct holder<T1, T2, T3, T4, T5>
{ 
    union { T1 t1; T2 t2; T3 t3; T4 t4; T5 t5; }; 

    holder() noexcept {}

    template<class... Args>
    holder(type_of<T1>, Args&&... args) 
        : t1(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T2>, Args&&... args) 
        : t2(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T3>, Args&&... args) 
        : t3(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T4>, Args&&... args) 
        : t4(std::forward<Args>(args)...)
    {}

    template<class... Args>
    holder(type_of<T5>, Args&&... args) 
        : t5(std::forward<Args>(args)...)
    {}

    ~holder() {}

    operator T1&() { return t1; }
    operator T2&() { return t2; }
    operator T3&() { return t3; }
    operator T4&() { return t4; }
    operator T5&() { return t5; }
};

// ...

// helper functions for switch cases
namespace cases
{

template<class T, class Union, class... Args>
bool constructor(type_code_t type, Union& u, Args&&... args)
{
    return switch_case(
        type, 
        type_of<T>::code(),
        [&u, &args...]()
        {
            new(&(T&)u) T(std::forward<Args>(args)...);
        }
    );
}

template<class T, class Union>
bool copy_constructor(type_code_t type, const Union& src, Union& dst)
{
    return switch_case(
        type, 
        type_of<T>::code(),
        [&src, &dst]()
        {
            new(&(T&)dst) T((const T&)src);
        }
    );
}

template<class T, class Union>
bool move_constructor(type_code_t type, Union& src, Union& dst)
{
    return switch_case(
        type, 
        type_of<T>::code(),
        [&src, &dst]()
        {
            new(&(T&)dst) T(std::move((T&)src));
        }
    );
}

// swap different types
template<class T1, class T2, class U/*, class = void*/>
struct swapper
{
    static void swap(U& a, U& b)
    {
        auto t = std::move((T2&)b);
        new(&(T1&)b) T1(std::move((T1&)a));
        new(&(T2&)a) T2(std::move(t));
    }
};

// the same non-void type - just swap
template<class T, class U>
struct swapper<T, T, U>
{
    static void swap(U& a, U& b) 
    { 
        using std::swap;
        swap((T&)a, (T&)b);
    }
};

// swap(void, void)
template<class U>
struct swapper<void, void, U>
{
    static void swap(U&, U&) {}
};

// swap(void, T)
template<class T, class U>
struct swapper<void, T, U>
{
    static void swap(U& a, U& b)
    {
        new(&(T&)a) T(std::move((T&)b));
    }
};

// swap(T, void)
template<class T, class U>
struct swapper<T, void, U>
{
    static void swap(U& a, U& b)
    {
        new(&(T&)b) T(std::move((T&)a));
    }
};

template<class T1, class T2, class Union>
bool swap(type_code_t tc2, Union& a, Union& b)
{
    return switch_case(
        tc2,
        type_of<T2>::code(),
        [&a, &b]()
        {
            swapper<T1, T2, Union>::swap(a, b);
        }
    );
}

template<class T1, class Union, class... Ts>
bool swap1(type_code_t tc1, type_code_t tc2, Union& a, Union& b)
{
    return switch_case(
        tc1, 
        type_of<T1>::code(),
        [tc2, &a, &b]()
        {
            if (!do_switch(
                tc2,
                std::forward_as_tuple(a, b),
                cases::swap<T1, Ts, Union>...
            ))
            {
                throw exception<type_error>(
                    "broken union in swap (2)"
                );
            }
        }
    );
}

template<class T, class Union>
bool destructor(type_code_t type, Union& u)
{
    return switch_case(
        type, 
        type_of<T>::code(),
        [&u]()
        {
            ((T&)u).~T();
        }
    );
}

template<class T, class TR, class Union>
bool cast(
    type_code_t type, 
    Union& u, 
    std::reference_wrapper<TR>& result
)
{
    static_assert(std::is_base_of<TR, T>::value, "it can never happen");

    return switch_case(
        type,
        type_of<T>::code(),
        [&u, &result]()
        {
            result = std::reference_wrapper<TR>((T&)u);
        }
    );
}

template<class Pack>
struct cast1;

template<class... Ts>
struct cast1<pack::type<Ts...>>
{
    template<class T, class Union>
    static bool do_switch(
        type_code_t type, 
        Union& u,
        std::reference_wrapper<T>& result
    )
    {
        return types::do_switch(
            type, 
            std::forward_as_tuple(u, result), 
            cases::cast<Ts, T, Union>...
        );
    }
};

} // cases

} // union_

template<class... Ts>
class safe_union
{
    typedef union_::holder<Ts...> union_type;

public:
    // void object - both values are uninitialized
    safe_union() noexcept : the_type(type_of<void>::code()) {}

    template<class T, class... Args>
    safe_union(type_of<T> t, Args&&... args)
        : the_type(t.code()),
            u(t, std::forward<Args>(args)...)
    {}

    // Dynamic constructor. Supports the Ts... and void types also.
    template<class... Args>
    safe_union(type_code_t type_code, Args&&... args) : the_type(type_code)
    {
        using namespace std;

        if (type_code == code<void>())
        {
                return;
        }

        if (!do_switch(
            type_code,
            forward_as_tuple(u, forward<Args>(args)...),
            union_::cases::constructor<Ts, union_type, Args&&...>...
        ))
        {
            throw types::exception<type_error>(
                "the type is not supported by the union"
            );
        }
    }

    safe_union(const safe_union& o) : the_type(o.type())
    {
        using namespace std;

        if (the_type == code<void>())
        {
            return;
        }

        if (!do_switch(
            the_type,
            forward_as_tuple(o.u, u),
            union_::cases::copy_constructor<Ts, union_type>...
        ))
        {
            throw types::exception<type_error>(
                "broken union in copy constructor"
            );
        }
    }

    safe_union(safe_union&& o) : the_type(o.type()) // TODO noexcept
    {
        using namespace std;

        if (the_type == code<void>())
        {
            return;
        }

        if (!do_switch(
            the_type,
            forward_as_tuple(o.u, u),
            union_::cases::move_constructor<Ts, union_type>...
        ))
        {
            throw types::exception<type_error>(
                "broken union in move constructor"
            );
        }
    }

    ~safe_union()
    {
        using namespace std;

        if (the_type == code<void>())
        {
            return;
        }

        do_switch(
            the_type,
            forward_as_tuple(u),
            union_::cases::destructor<Ts, union_type>...
        );
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

    void swap(safe_union& o)
    {
        if (!do_switch(
            the_type,
            std::forward_as_tuple(o.the_type, u, o.u),
            union_::cases::swap1<void, union_type, void, Ts...>,
            union_::cases::swap1<Ts, union_type, void, Ts...>...
        ))
        {
            throw exception<type_error>(
                "broken union is swap (1)"
            );
        }
        std::swap(the_type, o.the_type);
    }

    // Changes the stored type. It calls a destructor (if not
    // void) and then a constructor for a new type with args...
    // If new_type is the same as type() do nothing.
    template<class... Args>
    void reconstruct(type_code_t new_type, Args&&... args)
    {
        if (new_type != type()) 
        { 
            this->~safe_union();
            // be careful now, actually the object is funny
            // destroyed ... 

            new(this) safe_union(
                new_type, 
                std::forward<Args>(args)...
            );
        }
    }

    template<class T, class... Args>
    void static_reconstruct(Args&&... args)
    {
        reconstruct(type_of<T>::code(), std::forward<Args>(args)...);
    }

    // Doesn't consider base types
    template<class T>
    bool contains() const
    {
        // NB std::decay
        return the_type == 
          type_of<typename std::decay<T>::type>::code();
    }

    // a type of current union
    type_code_t type() const { return the_type; }

    // type dictionary
    template<class T>
    static type_code_t code() { return type_of<T>::code(); }

    template<class T>
    operator T&()
    {
        typedef typename std::remove_const<T>::type T1;
        T1& nullptr_ref = *(T1*)nullptr;
        std::reference_wrapper<T1> result(nullptr_ref);

        // try to cast to some descendant of T. 
        // (T is also a descendant of T here)
        if (union_::cases::cast1<
                typename types::select_descendants<T, Ts...>
                  ::descendants
            > :: do_switch(the_type, u, result))
        {
            return result;
        }
        else
        {
            throw exception<type_error>(
                "unable to cast the union of the type ", 
                limit<64, limit_policy::get_tail>(
                    mangled_name<mixed_string>(the_type)
                ),
                " to the type ",
                limit<64, limit_policy::get_tail>(
                    type_of<T>::template mangled_name<mixed_string>()
                )
            );
        }
    }
        
    template<class T>
    operator const T&() const
    {
        return const_cast<safe_union*>(this)->operator T&();
    }
    
protected:
    safe_union(int, type_code_t type_code) : the_type(type_code) {}

    type_code_t the_type;
    union_type u;
};

template<class... Ts>
void swap(safe_union<Ts...>& a, safe_union<Ts...>& b)
{
    a.swap(b);
}

} // types

#endif
