/**
 * @file
 * enum-like constexpr class.
 * You can easily convert between int value of enum and string name.
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

#include "maps.h"
#include "types/typeinfo.h"
#include <cassert>
#include <functional>
#include <ios>
#include <iterator>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifndef TYPES_ENUM_H
#define TYPES_ENUM_H

namespace enumerate {

namespace enum_ {

template<class Int, class Base, Int MaxRange, class... Vals>
class base;

//! get a name of enum value
template<class EnumVal, class String = std::string>
String get_name()
{
  auto type_name = types::type_of<EnumVal>::template name<String>();
  // select only type name, discard namespace
  auto pos = type_name.find_last_of(':');
  if (pos != decltype(type_name)::npos)
    return type_name.substr(pos + 1);
  else
    return type_name;
}

template<class Int, class Base>
struct dict
{
	using int_type = Int;
	using range_type = std::pair<int_type, int_type>;
	using pointer_type = const Base*; //std::shared_ptr<const Base>;
	using string = marker::type<marker::non_empty_string_marker, std::string>;

	struct interval_size_marker : marker::integer_marker<int_type, 0> {};
	using interval_size_type = marker::type<interval_size_marker, int_type>;
	
  using dictionary = map::multi_way_object_indexer::type<
		map::unordered_map,
		map::deque,
		int_type,
		std::tuple<string, int_type>,
	  std::tuple<pointer_type, interval_size_type>
	>;

	using index_type = typename dictionary::index_marker_type;
	
	template<class Val>
	static auto make_row(
		const std::string& name,
		int_type idx,
		interval_size_type interval_size
	)
	{
    return std::make_tuple(
			string::cast(name),
			idx,
			pointer_type{new Val}, // TODO make Val as static var
			interval_size
		); 
	}
	
	static pointer_type base_ptr(const dictionary& d, int_type i)
	{
		return d[i].template by_type<const pointer_type>(); 
  }
	
	static range_type range(const dictionary& d, int_type i)
	{
		auto interval_size = d[i].template by_type<const interval_size_type>();
		int_type interval_size_val;
		if (!types::get_value(interval_size, interval_size_val))
			return range_type{i, i};
		
		return range_type{
			i,
			i + interval_size_val - 1
		};
  }
};

template<class Int>
struct dict<Int, void>
{
	using int_type = Int;
	using range_type = std::pair<int_type, int_type>;
	using pointer_type = void;
	using string = marker::type<marker::non_empty_string_marker, std::string>;

	struct interval_size_marker : marker::integer_marker<int_type, 0> {};
	using interval_size_type = marker::type<interval_size_marker, int_type>;

  using dictionary = map::multi_way_object_indexer::type<
		map::unordered_map,
		map::deque,
		int_type,
		std::tuple<string, int_type>,
	  std::tuple<interval_size_type>
	>;

	template<class Val>
	static auto make_row(
		const std::string& name,
		int_type idx,
		interval_size_type interval_size
	)
	{
    return std::tuple<decltype(string::cast(name)), int_type, interval_size_type>{
			string::cast(name),
			idx,
			interval_size
		};
	}

	static range_type range(const dictionary& d, int_type i)
	{
		auto interval_size = d[i].template by_type<const interval_size_type>();
		int_type interval_size_val;
		if (!types::get_value(interval_size, interval_size_val))
			return range_type{i, i};
		
		return range_type{
			i,
			i + interval_size_val - 1
		};
  }

	static const void* base_ptr(const dictionary& d, int_type i)
	{
		assert(false); // should never be called
		return nullptr;
  }
};

template<class EnumVal, class Index, Index N, class Enable = void>
struct enum_const_def
{
	using range_type = std::pair<Index, Index>;
	
	static constexpr Index index() { return N; }
	static constexpr range_type range() { return range_type{N, N}; }
};

template<class EnumVal, class Index, Index N>
struct enum_const_def<EnumVal, Index, N, types::void_t<decltype(EnumVal::c())>>
{
	using range_type = std::pair<Index, Index>;
	
	static constexpr Index index() { return EnumVal::c(); }
	static constexpr range_type range() { return range_type{index(), index()}; }
};

template<class EnumVal, class Index, Index N>
struct enum_const_def<EnumVal, Index, N, types::void_t<decltype(EnumVal::range())>>
{
	using range_type = std::pair<Index, Index>;
	
	static constexpr Index index() { return range().first; }
	static constexpr range_type range() { return EnumVal::range(); }
};

//! The enum meta information type
template<class Int, Int MaxRange, class Base, Int N, class... Vals>
class meta;

template<class Int, Int MaxRange, class Base, Int N>
class meta<Int, MaxRange, Base, N>
{
  template<class I, class T, class... V>
  friend class enum_::base;

public:
  using int_type = Int;
	
protected:
  static constexpr Int n = enum_const_def<void, Int, N>::index();

public:
	template<class String = std::string>
  static const String& name() 
  {
		static String str{""};
    return str;
	}

  static constexpr Int meta_index()
  {
    return n;
  }

protected:
  template<class It>
  static void fill_dict(It) 
  {
  }
};

template<class Int, Int MaxRange, class Base, Int N, class Val, class... Vals>
class meta<Int, MaxRange, Base, N, Val, Vals...>
	: public meta<Int, MaxRange, Base, N, Vals...>
{
  template<class I, class T, class... V>
  friend class enum_::base;

  using base = meta<Int, MaxRange, Base, N, Vals...>;

public:
	using int_type = Int;
	using range_type = std::pair<int_type, int_type>;
	
protected:
	using const_def = enum_const_def<Val, Int, base::n - 1>;
	
  static constexpr range_type the_range = const_def::range();
  static constexpr int_type n = the_range.first;

	static constexpr int_type range_size()
	{
		return the_range.second - the_range.first + 1;
	}

	static_assert(the_range.second >= the_range.first, "enum: bad range definition");
	static_assert(range_size() <= MaxRange, "enum: too wide range");
	
public:
  using base::name;
  using base::meta_index;

  template<class String = std::string>
  static const String& name(const Val&) 
  {
    static String the_name = get_name<Val, String>();
    return the_name;
  }

protected:
  template<class It>
  static void fill_dict(It it)
  {
		using the_dict = dict<Int, Base>;
		
		*it++ = the_dict::template make_row<Val>(
			name(*(Val*)0),
			n,
			typename the_dict::interval_size_type(
				the_range.second - the_range.first + 1
			)
		);
		base::fill_dict(it);
  }

  static constexpr Int meta_index(const Val&)
  {
    return n;
  }
};

static int xalloc()
{
  static int xalloc_ = std::ios_base::xalloc();
  return xalloc_;
}

//! Contains the types array
// TODO: make the specialization Base = void
template<class Int, class Base, Int MaxRange, class... Vals>
class base
{
public:
  using int_type = Int;
	using range_type = std::pair<int_type, int_type>;
	using dict_helper = enum_::dict<Int, Base>;
	using dictionary = typename dict_helper::dictionary;
	using pointer_type = typename dict_helper::pointer_type;
	using index_type = typename dictionary::index_marker_type;

public:
	static bool is_valid_index(int_type idx)
	{
    const auto& d = dict();

		return d.find(idx) != d.end();
	}

	static const std::string& name(int_type i)
  {
    const auto& d = dict();

		return d[i].template by_type<const typename dict_helper::string>();
  }

	static pointer_type base_ptr(int_type i)
	{
		return dict_helper::base_ptr(dict(), i);
  }

	static range_type range(int_type i)
	{
		return dict_helper::range(dict(), i);
	}
	
  template<int_type NotFound, class String>
  static int_type lookup(const String& s)
  {
    const auto& indexes = dict();
    const auto it = indexes.find(dict_helper::string::cast(s));
    return (it != indexes.end()) ? types::strip((*it).index(), NotFound) : NotFound;
  }

private:
  static dictionary& dict() 
  {
    static dictionary the_dict = build_dictionary();
    return the_dict;
  }

  static dictionary build_dictionary()
  {
    dictionary d;
    // TODO d.reserve(sizeof...(Vals));
    meta<Int, MaxRange, Base, sizeof...(Vals), Vals...>
      ::fill_dict(map::back_inserter(d));
    return d;
  }
};

} // enum_


/* =======================[   enumerate   ]======================== */

template<class Int, class Base, Int MaxRange, class... Vals>
class type_with_base
	: protected enum_::meta<Int, MaxRange, Base, sizeof...(Vals), Vals...>,
    protected enum_::base<Int, Base, MaxRange, Vals...>
{
  using base = enum_::base<Int, Base, MaxRange, Vals...>;

protected:
  using meta = enum_::meta<Int, MaxRange, Base, sizeof...(Vals), Vals...>;
	
public:
  using meta::name;

  constexpr type_with_base() : idx(bottom_idx()) {}

  template<
    class EnumVal,
    decltype(meta::meta_index(*(EnumVal*)0)) = 0
  >
  constexpr type_with_base(const EnumVal& val)
		: idx(meta::meta_index(val))
	{}

	type_with_base& assign(const std::string& name)
	{
		parse(name);
		return *this;
	}
	
	type_with_base& assign(Int i)
	{
		idx = base::is_valid_index(i) ? i : bottom_idx();
		return *this;
	}
	
  const std::string& name() const
  {
    if (__builtin_expect(idx != bottom_idx(), 1))
    {
        return base::name(idx);
    }
    else
    {
        static std::string na = "<N/A>";
        return na;
    }
	}

	constexpr Int index() const
	{
		return idx;
	}

	bool get_as_string(std::string& name, std::nullptr_t) const
	{
    if (__builtin_expect(idx != bottom_idx(), 1))
    {
			name = base::name(idx);
			return true;
		}
		else
			return false;
	}

  template<class String>
  void parse(const String& s)
  {
    idx = base::template lookup<bottom_idx()>(s);
  }

  constexpr bool operator==(const type_with_base& b) const
  {
    return idx == b.idx;
  }

  template<
    class E2,
    decltype(meta::meta_index(E2())) = 0
  >
  constexpr bool operator==(const E2& b) const
  {
    return meta::meta_index(b) == idx;
  }

  template<class T>
  constexpr bool operator!=(const T& b) const
  {
    return ! operator==(b);
  }

  static constexpr typename meta::int_type size()
  {
    return sizeof...(Vals);
  }

#if 0 // has no sence because could be nullptr
	const Base* operator->() const
	{
		return base::base_ptr(idx);
	}
#else
	const Base* object_ptr() const
	{
		return base::base_ptr(idx);
	}
#endif

// protected: //FIXME return to protected
  // means "NA"
  constexpr static Int bottom_idx()
	{
		return std::numeric_limits<Int>::max();
	}

  Int idx;
	//Base* base_ptr;

	bool in(Int first, Int i) const
	{
		const auto r = base::range(first);
		return i >= r.first && i <= r.second;
	}

  // It is protected to allow safely build
  // template<class T> method accepting
  // the type_with_base in different form
  // supported by public constructors
  // e.g. string, EnumVal type, enumerate etc.
  explicit type_with_base(Int i) : idx(i)
	{
		for (Int cnt = 0;
				 !base::is_valid_index(idx) && cnt < MaxRange;
				 --idx, cnt++)
			;	// search the start of the range

		if (!base::is_valid_index(idx) || !in(idx, i))
			idx = bottom_idx();
	}
};

template<class A, class Int, class Base, Int MaxRange, class... Vals>
bool operator==(A&& a, const type_with_base<Int, Base, MaxRange, Vals...>& b)
{
	return b.operator==(std::forward<A>(a));
}

template<class Int, class Base, Int MaxRange, class... Vals>
std::istream& operator>>(std::istream& in, type_with_base<Int, Base, MaxRange, Vals...>& val)
{
	std::string str;
	in >> str;
	val.parse(str);
	return in;
}


template<class Int, class... Vals>
using type = type_with_base<Int, void, 1, Vals...>;


//! The enumerate class which is convertible to Int type
template<class Int, class Base, class... Vals>
class convertible_with_base : public type_with_base<Int, Base, 1, Vals...>
{
	using base = type_with_base<Int, Base, 1, Vals...>;

public:
  using int_type = Int;
  using base::meta_index;

  constexpr convertible_with_base()  {}

  template<class EnumVal>
  constexpr convertible_with_base(const EnumVal& val) : base(val) {}

  constexpr convertible_with_base(int_type i) : base(i) {}

  /*static convertible_with_base from_index(Int i)
  {
    return convertible_with_base(i);
		}*/

  constexpr int_type index() const
  {
    return this->idx;
  }

  /*template<class String>
  static convertible_with_base parse(const String& s)
  {
    return convertible_with_base(base::template lookup<base::bottom_idx()>(s));
		}*/

  static constexpr int_type min()
  {
    return 0;
  }

  static constexpr int_type max()
  {
    return (int_type) base::size() - 1;
  }

  explicit constexpr operator int_type() const
  {
    return index();
  }

	/*  !!! conversion (bottom value) danger, don't use this method
	bool operator==(int_type idx_b) const
	{
		return index() == idx_b;
		}*/

	using base::operator==;
};

template<class Int, class... Vals>
using convertible = convertible_with_base<Int, void, Vals...>;

static_assert(
	sizeof(convertible_with_base<char, std::thread, char, int>) == sizeof(char),
	"cannot reinterpret cast from Int to enumerate::type"
);

template<class A, class Int, class Base, class... Vals>
bool operator==(A&& a, const convertible_with_base<Int, Base, Vals...>& b)
{
	return b.operator==(std::forward<A>(a));
}

//! The enumerate class which can contain ranges
template<class Int, class Base, Int MaxRange, class... Vals>
class ranged_with_base : public type_with_base<Int, Base, MaxRange, Vals...>
{
	using base = type_with_base<Int, Base, MaxRange, Vals...>;

protected:
	using typename base::meta;

	Int range_first_idx = base::bottom_idx();
	
public:
  using int_type = Int;
	using range_type = std::pair<int_type, int_type>;
  using base::meta_index;

  constexpr ranged_with_base()
	{
		range_first_idx = this->idx;
	}

  template<class EnumVal>
  constexpr ranged_with_base(const EnumVal& val) : base(val)
	{
		range_first_idx = this->idx;
	}

  explicit ranged_with_base(int_type i) : base(i)
	{
		range_first_idx = this->idx;
		if (this->idx != i && in(i))
			this->idx = i;
	}

  const std::string& name() const
  {
		return base{range_first_idx}.name();
	}

  range_type range() const
  {
    return base::range(range_first());
  }

  constexpr int_type range_first() const
  {
    return range_first_idx;
  }

	bool in(Int i) const
	{
		return base::in(range_first(), i);
	}
	
	/* use operator== instead
	bool in(const base& o) const
	{
		return in(o.index());
	}
	*/
	
  constexpr bool operator==(const ranged_with_base& b) const
  {
    return range_first() == b.range_first();
  }

  template<
    class E2,
    decltype(meta::meta_index(E2())) = 0
  >
  constexpr bool operator==(const E2& b) const
  {
    return ranged_with_base{b}.range_first() == range_first();
  }

  template<class T>
  constexpr bool operator!=(const T& b) const
  {
    return ! operator==(b);
  }
};

template<class Int, class... Vals>
using ranged = ranged_with_base<Int, void, 20, Vals...>;

#if 0
// NB
static_assert(
	sizeof(ranged_with_base<char, std::thread, 20, char, int>) == sizeof(char),
	"cannot reinterpret cast from Int to enumerate::type"
);
#endif

template<class A, class Int, class Base, Int MaxRange, class... Vals>
bool operator==(A&& a, const ranged_with_base<Int, Base, MaxRange, Vals...>& b)
{
	return b.operator==(std::forward<A>(a));
}

// i.e. enum_type_index<red>() or enum_type_index(red())
template<class EnumVal>
struct enum_type_index
{
  enum_type_index() {}
  enum_type_index(EnumVal) {}

  operator std::type_index() const
  {
    return std::type_index(typeid(EnumVal));
  }
};

#if 0
// i.e. colour = red(); enum_type_index(colour);
template<class Int, class... Vals>
struct enum_type_index<enumerate<Int, Vals...>>
{
  // TODO
};
#endif

//! Switch printing enums as strings or integers. The
//! string mode is default
inline std::ios_base& enumalpha(std::ios_base& ios)
{
  ios.iword(enum_::xalloc()) = 0;
  return ios;
}

inline std::ios_base& noenumalpha(std::ios_base& ios)
{
  ios.iword(enum_::xalloc()) = 1;
  return ios;
}

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Int,
	class Base,
	Int MaxRange,
  class... EnumVals
>
std::basic_ostream<CharT, Traits>&
operator << ( 
  std::basic_ostream<CharT, Traits>& out,
  type_with_base<Int, Base, MaxRange, EnumVals...> v
)
{
    try
    {
      out << v.name();
    }
    catch (...)
    {
      out << '*';
    }
    return out;
}

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Int,
	class Base,
  class... EnumVals
>
std::basic_ostream<CharT, Traits>&
operator << ( 
  std::basic_ostream<CharT, Traits>& out,
  convertible_with_base<Int, Base, EnumVals...> v
)
{
  if (out.iword(enum_::xalloc()))
  {
    out << (intmax_t) v.index(); // prevent printing 
                                 // int8_t as char
  }
  else
  {
    try
    {
      out << v.name();
    }
    catch (...)
    {
      out << '*';
    }
  }
  return out;
}

template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Int,
	class Base,
	Int MaxRange,
  class... EnumVals
>
std::basic_ostream<CharT, Traits>&
operator << ( 
  std::basic_ostream<CharT, Traits>& out,
  ranged_with_base<Int, Base, MaxRange, EnumVals...> v
)
{
  if (out.iword(enum_::xalloc()))
  {
    out << (intmax_t) v.index(); // intmax_t - prevent printing 
                                 // int8_t as char
  }
  else
  {
    try
    {
      out << v.name() << '(' << (intmax_t) v.index()  << ')';
    }
    catch (...)
    {
      out << '*';
    }
  }
  return out;
}

#if 0
template <
  class CharT,
  class Traits = std::char_traits<CharT>,
  class Int,
  class... EnumVals
>
std::basic_istream<CharT, Traits>&
operator >> ( 
  std::basic_istream<CharT, Traits>& in, 
  enumerate<Int, EnumVals...>& e 
)
{
  if (in.iword(EnumBase::xalloc)) {
    std::string name;
    in >> name;
    e = enum_t<T, N, Int>(name.c_str());
  }
  else in >> e.value;
  return in;
}
#endif

} // namespace enumerate

#endif


