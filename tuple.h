#pragma once

#include "types/typeinfo.h"
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <tuple>
#include <unordered_map>

namespace types
{

// This is a special reference which is used the same way as a null pointer
template<class T>
struct zero_ref_base
{
	static T zero_ref_obj;

	static bool check(T& o)
	{
		return &zero_ref_obj == &o;
	}

	static bool check(const T& o)
	{
		return &zero_ref_obj == &o;
	}
};

template<class T>
T zero_ref_base<T>::zero_ref_obj;

template<class T>
struct zero_ref : zero_ref_base<T>
{
	using base = zero_ref_base<T>;
	
	static T& value()
	{
		return base::zero_ref_obj;
	}

	static std::reference_wrapper<T> ref_wrapper()
	{
		return std::ref(value());
	}
};

template<class U>
struct zero_ref<const U> : zero_ref_base<U>
{
	using base = zero_ref_base<U>;
	
	static const U& value()
	{
		return base::zero_ref_obj;
	}

	static std::reference_wrapper<const U> ref_wrapper()
	{
		return std::cref(value());
	}
};

template<class T>
struct zero_ref<T&> : zero_ref_base<T>
{
	using base = zero_ref_base<T>;
	
	static T& value()
	{
		return base::zero_ref_obj;
	}

	static std::reference_wrapper<T> ref_wrapper()
	{
		return std::ref(value());
	}
};

template<class U>
struct zero_ref<const U&> : zero_ref_base<U>
{
	using base = zero_ref_base<U>;
	
	static const U& value()
	{
		return base::zero_ref_obj;
	}

	static std::reference_wrapper<const U> ref_wrapper()
	{
		return std::cref(value());
	}
};

template<class U>
struct zero_ref<std::reference_wrapper<U>> : zero_ref<U>
{
	using base = zero_ref<U>;
	
	static std::reference_wrapper<U> value()
	{
		return base::ref_wrapper();
	}

	static std::reference_wrapper<U> ref_wrapper()
	{
		return value();
	}
};

template<class U>
struct zero_ref<std::reference_wrapper<const U>> : zero_ref<const U>
{
	using base = zero_ref<const U>;
	
	static std::reference_wrapper<const U> value()
	{
		return base::ref_wrapper();
	}

	static std::reference_wrapper<const U> ref_wrapper()
	{
		return value();
	}
};

// std::remove_cvref appears only in C++20
template<class T>
struct remove_cvref
{
	using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template<class T>
using remove_cvref_t = typename remove_cvref<T>::type;
	
// remove ref and reference_wrapper
template<class T, class Enable = void>
struct remove_refw
{
	using type = T;
};

template<class T>
struct remove_refw<std::reference_wrapper<T>>
{
	using type = typename remove_refw<T>::type;
};

template<class T>
struct remove_refw<
	T,
	typename std::enable_if<std::is_reference<T>::value, void>::type
>
{
	using type = std::remove_reference_t<T>;
};

template<class T>
using remove_refw_t = typename remove_refw<T>::type;

// remove cv, ref and reference_wrapper
template<class T>
struct remove_cvrefw
{
	using type = std::remove_cv_t<types::remove_refw_t<T>>;
};

template<class T>
using remove_cvrefw_t = typename remove_cvrefw<T>::type;


template<class U>
static auto unwrap(const std::reference_wrapper<U>& u)
{
	return u.get();
}

template<class T>
static constexpr auto unwrap(const T& v)
{
	return v;
}

#if 0
template<class T>
struct remove_c_from_ptr
{
	using type = T;
};

template<class T>
struct remove_c_from_ptr<std::unique_ptr<T, std::default_delete<T>>>
{
	using type = std::unique_ptr<std::remove_const_t<T>>;
};

template<class T>
struct decay_cptr
{
	using type = typename remove_c_from_ptr<std::decay_t<T>>::type;
};
#endif

template<class V>
struct key_type {};

template<class Key, class Value, class... Args>
struct key_type<std::unordered_map<Key, Value, Args...>>
{
	using type = Key;
};

template<class Key, class Value, class... Args>
struct key_type<std::map<Key, Value, Args...>>
{
	using type = Key;
};

template<class Key, class Value, class... Args>
struct key_type<std::multimap<Key, Value, Args...>>
{
	using type = Key;
};

template<class Value, class... Args>
struct key_type<std::deque<Value, Args...>>
{
	using type = Value;
};

template<class Container>
struct key_type<const Container> : key_type<Container>
{};

template<class T>
struct retempl {};

template<template <class> class Templ, class T>
struct retempl<Templ<T>>
{
	template<class U>
	using templ = Templ<U>;
};

} // namespace types

namespace tuple
{

namespace impl_
{

template<class... Fields>
std::tuple<Fields...> as_tuple_dummy_fun(const std::tuple<Fields...>& obj);

template<class...>
void as_tuple_dummy_fun(...);

} // namespace impl_

template<class Object>
struct as_tuple
{
	using type = decltype(impl_::as_tuple_dummy_fun(std::declval<Object>()));
};

template<class T>
struct has_tuple_as_base
	: std::integral_constant<bool, !std::is_same<typename as_tuple<T>::type, void>::value>
{};

// std::make_tuple unwrap reference_wrapper-s, this implementation doesn't
namespace preserve_wrappers
{

template<class... Ts>
auto make(Ts&&... args)
{
	using tuple = std::tuple<typename std::decay<Ts>::type...>;
	
	return tuple(std::forward<Ts>(args)...);
}

} // namespace preserve_wrappers

template<class TupleA, class TupleB>
struct cat
{
	using type = decltype(
		std::tuple_cat(std::declval<TupleA>(), std::declval<TupleB>())
	);
};
	
template<class Tuple, template <class> class Mod>
struct type_mod_for_each {};

template<class T0, class... Ts, template <class> class Mod>
struct type_mod_for_each<std::tuple<T0, Ts...>, Mod>
{
	using type = typename cat<
		std::tuple<typename Mod<T0>::type>,
		typename type_mod_for_each<std::tuple<Ts...>, Mod>::type
	>::type;
};

#if 0
#if 0
template<class Tuple, template <class> class Mod>
struct type_mod_for_each<const Tuple, Mod> : type_mod_for_each<Tuple, Mod>
{};
#else
template<class T0, class... Ts, template <class> class Mod>
struct type_mod_for_each<const std::tuple<T0, Ts...>, Mod>
{
	using type = typename cat<
		std::tuple<typename Mod<T0>::type>,
		typename type_mod_for_each<std::tuple<Ts...>, Mod>::type
	>::type;
};
#endif
#endif

template<template <class> class Mod>
struct type_mod_for_each<std::tuple<>, Mod>
{
	using type = std::tuple<>;
};

template<class Tuple, template <class> class Mod>
using type_mod_for_each_t = typename type_mod_for_each<Tuple, Mod>::type;


// TODO rewrite following using type_mod_for_each

template<class Tuple>
struct unref {};

template<class T0, class... Ts>
struct unref<std::tuple<std::reference_wrapper<T0>, Ts...>>
{
	using type = typename cat<std::tuple<T0>, typename unref<std::tuple<Ts...>>::type>::type;
};

template<class T0, class... Ts>
struct unref<std::tuple<T0, Ts...>>
{
	using type = typename cat<std::tuple<T0>, typename unref<std::tuple<Ts...>>::type>::type;
};

template<>
struct unref<std::tuple<>>
{
	using type = std::tuple<>;
};


template<class Tuple>
struct unconst {};

template<class T0, class... Ts>
struct unconst<std::tuple<const T0, Ts...>>
{
	using type = typename cat<std::tuple<T0>, typename unconst<std::tuple<Ts...>>::type>::type;
};

template<class T0, class... Ts>
struct unconst<std::tuple<T0, Ts...>>
{
	using type = typename cat<std::tuple<T0>, typename unconst<std::tuple<Ts...>>::type>::type;
};

template<>
struct unconst<std::tuple<>>
{
	using type = std::tuple<>;
};

template<class Tuple>
struct addconst {};

template<class T0, class... Ts>
struct addconst<std::tuple<T0, Ts...>>
{
	using type = typename cat<std::tuple<const T0>, typename addconst<std::tuple<Ts...>>::type>::type;
};

template<>
struct addconst<std::tuple<>>
{
	using type = std::tuple<>;
};


template<class A, class B, std::size_t Level, class Enable = void>
struct comparator {};

template<
	class A,
	class B,
	std::size_t Level
>
struct comparator<
	A, B, Level,
	typename std::enable_if<std::tuple_size<A>::value != std::tuple_size<B>::value>::type
>
{
	static constexpr bool equal(const A& a, const B& b)
	{
		return false;
	}
};

template<
	class A,
	class B,
	std::size_t Level
>
struct comparator<
	A, B, Level,
	std::enable_if_t<
		std::tuple_size<A>::value == std::tuple_size<B>::value
		&& Level < std::tuple_size<A>::value
	>
>
{
	static bool equal(const A& a, const B& b)
	{
		return types::unwrap(std::get<Level>(a)) == types::unwrap(std::get<Level>(b))
		  && comparator<A, B, Level + 1>::equal(a, b);
	}
};

template<
	class A,
	class B,
	std::size_t Level
>
struct comparator<
	A, B, Level,
	std::enable_if_t<
		std::tuple_size<A>::value == std::tuple_size<B>::value
		&& Level >= std::tuple_size<A>::value
	>
>
{
	static constexpr bool equal(const A& a, const B& b)
	{
		return true;
	}
};


template<class T, class... Us>
struct among_types {};

template<class T>
struct among_types<T> : std::integral_constant<bool, false> {};

template<class T, class... Us>
struct among_types<T, T, Us...> : std::integral_constant<bool, true> {};

template<
	class T,
	class U,
	class... Us
>
struct among_types<
	T,
	U,
	Us...
	> : among_types<T, Us...> {};

template<class T, class Tuple>
struct among_tuple_types {};

template<class T, class... Us>
struct among_tuple_types<T, std::tuple<Us...>> : among_types<T, Us...> {};

// select T from Us... as tuple<T> or tuple<> if there is no T among Us...
template<class T, class... Us>
struct select1 {};

template<class T>
struct select1<T>
{
	using type = std::tuple<>;
};

template<class T, class... Us>
struct select1<T, T, Us...>
{
	using type = std::tuple<T>;
};

template<
	class T,
	class U,
	class... Us
>
struct select1<
	T,
	U,
	Us...
	> : select1<T, Us...> {};


template<class Tuple, std::size_t Level>
struct recursive_helper {};

template<class T0, class... Ts, std::size_t Level>
struct recursive_helper<std::tuple<T0, Ts...>, Level> : recursive_helper<std::tuple<Ts...>, Level + 1>
{
	using base = recursive_helper<std::tuple<Ts...>, Level + 1>;

	template<class Tuple>
	static void out(std::ostream& s, const Tuple& tup, std::string delim = ", ")
	{
		if (Level > 0)
			s << delim;
		out_element(s, std::get<T0>(tup));
		base::out(s, tup);
	}

	template<class U>
	static void out_element(std::ostream& s, const std::reference_wrapper<U> u)
	{
		out_element(s, u.get());
	}
	
	template<class E>
	static auto out_element(std::ostream& s, const E& u)
		-> std::enable_if_t<std::is_enum<E>::value, void>
	{
		s << (int) u;
	}
	
	template<class Tup>
	static auto out_element(std::ostream& s, const Tup& tup)
		-> std::enable_if_t<tuple::has_tuple_as_base<Tup>::value, void>
	{
		using sub_tuple_type = typename tuple::as_tuple<Tup>::type;
		tuple::recursive_helper<sub_tuple_type, 0>::out(s, tup);
	}
	
	template<class U>
	static auto out_element(std::ostream& s, const U& u)
		-> std::enable_if_t<
			!std::is_enum<U>::value && !tuple::has_tuple_as_base<U>::value,
		  void
		>
	{
		s << u;
	}
	
	template<class A, class B>
	static void out_element(std::ostream& s, const std::pair<A, B>& pair)
	{
		s << '(';
		out_element(s, pair.first);
		s << ' ';
		out_element(s, pair.second);
		s << ')';
	}
	
	template<class T>
	static void out_element(std::ostream& s, const std::promise<T>& p)
	{
		s << "std::promise{...}";
	}
	
	template<class T>
	static void out_element(std::ostream& s, const std::shared_ptr<T>& p)
	{
		s << "std::shared_ptr{";
		out_element(s, p.get());
		s << '}';
	}
	
	template<class T>
	static void out_element(std::ostream& s, const std::list<T>& c)
	{
		s << "std::list{";
		bool first = true;
		for (const auto& el : c)
		{
			if (!first)
				s << ", ";
			else
				first = false;
			
			out_element(s, el);
		}
		s << '}';
	}
	
	template<class Key, class Value>
	static void out_element(std::ostream& s, const std::map<Key, Value>& c)
	{
		s << "std::map{";
		bool first = true;
		for (const auto& el : c)
		{
			if (!first)
				s << ", ";
			else
				first = false;
			
			out_element(s, el.first);
			s << "->";
			out_element(s, el.second);
		}
		s << '}';
	}
	
	template<class Clock, class Duration>
	static void out_element(std::ostream& s, const std::chrono::time_point<Clock, Duration>& tp)
	{
		s << put_time(tp, "%T");
		s << "@" << types::type_name<Clock>();
	}
	
	template<class Rep, class Period>
	static void out_element(std::ostream& s, const std::chrono::duration<Rep, Period>& dur)
	{
		s << dur.count() << "[" << Period::num << "/" << Period::den << "]";
	}
	
	template<
		class U,
		class Tuple,
		typename std::enable_if<
			std::is_const<U>::value
			&& std::is_same<types::remove_cvrefw_t<U>, types::remove_cvrefw_t<T0>>::value,
			bool
		>::type = false
	>
	constexpr static const T0& get_by_unref_type(Tuple& tup)
	{
		return std::get<Level>(tup);
	}

	template<
		class U,
		class Tuple,
		typename std::enable_if<
			!std::is_const<U>::value && !std::is_const<T0>::value
			&& std::is_same<types::remove_cvrefw_t<U>, types::remove_cvrefw_t<T0>>::value,
			bool
		>::type = false
	>
	constexpr static T0& get_by_unref_type(Tuple& tup)
	{
		return std::get<Level>(tup);
	}

	template<
		class U,
		class Tuple,
		typename std::enable_if<
			!std::is_const<U>::value && !std::is_const<T0>::value
			&& std::is_same<types::remove_cvrefw_t<U>, types::remove_cvrefw_t<T0>>::value,
			bool
		>::type = false
	>
	constexpr static const T0& get_by_unref_type(const Tuple& tup)
	{
		return std::get<Level>(tup);
	}

	template<
		class U,
		class Tuple,
		typename std::enable_if<
			!std::is_const<U>::value && std::is_const<T0>::value
			&& std::is_same<types::remove_cvrefw_t<U>, types::remove_cvrefw_t<T0>>::value,
			bool
		>::type = false
	>
	// the error here means that you try to access a const field by a non-const type
	constexpr static typename types::check_type<Tuple>::fail& get_by_unref_type(Tuple& tup)
	{
	}

	using base::get_by_unref_type;


	/* get_or_default
	 * Returns a tuple element by type (compares tuple elements with remove_const and remove_refw
	 * application) or a default value for the type if the tuple doesn't contain it
	 */
	
	template<
		class U,
		class Tuple,
		typename std::enable_if<
			std::is_same<types::remove_cvrefw_t<U>, types::remove_cvrefw_t<T0>>::value
			|| std::is_same<std::decay_t<U>, std::decay_t<T0>>::value,
			// the second condition is to be sure static_assert in expand() always detects mismatch
			bool
		>::type = false
	>
	constexpr static auto get_or_default(Tuple&& tup)
	{
		return std::get<Level>(std::forward<Tuple>(tup));
	}

	using base::get_or_default;

	template<class... Us>
	constexpr static bool each_among_types()
	{
		return among_types<T0, Us...>::value && base::template each_among_types<Us...>();
	}

#if 0
	// intersects T0, Ts... and Us...
	template<class... Us>
	constexpr static auto intersect()
	{
		return typename cat<
			typename select1<T0, Us...>::type,
			typename base::intersect<Us...>::type
		>::type;
	}
#endif
};

template<std::size_t Level>
struct recursive_helper<std::tuple<>, Level>
{
	template<class Tuple>
	static void out(std::ostream&, const Tuple&) {}
	
	template<
		class U,
		class Tuple
	>
	constexpr static void get_by_unref_type(Tuple&)
	{
	}

	template<
		class U,
		class Tuple
	>
	constexpr static U get_or_default(const Tuple&)
	{
		return U();
	}

	template<class... Us>
	constexpr static bool each_among_types()
	{
		return true;
	}

#if 0
	template<class... Us>
	constexpr static auto intersect()
	{
		return std::tuple<>;
	}
#endif
};

template<class Tuple>
struct helper {};

template<class Tuple>
struct helper0 {};

template<class... Ts>
struct helper0<std::tuple<Ts...>>
{
	using unref = typename tuple::unref<std::tuple<Ts...>>::type;
	
	using unconst = typename tuple::unconst<std::tuple<Ts...>>::type;
	
	template<template<class> class V>
	using tuple_of_containers = std::tuple<V<Ts>...>;

	template<template<class> class V>
	using tuple_of_ref_containers = std::tuple<V<std::reference_wrapper<Ts>>...>;

	template<template<class> class V>
	using tuple_of_cref_containers = std::tuple<V<std::reference_wrapper<const Ts>>...>;

	template<template<class, class> class Map, class CommonValueType>
	using tuple_of_maps = std::tuple<Map<Ts, CommonValueType>...>;

	template<template<class, class> class Map, class CommonValueType>
	using tuple_of_ref_maps = std::tuple<Map<std::reference_wrapper<Ts>, CommonValueType>...>;

	template<template<class, class> class Map, class CommonValueType>
	using tuple_of_cref_maps = std::tuple<Map<std::reference_wrapper<const Ts>, CommonValueType>...>;

	using wrap_crefs = std::tuple<std::reference_wrapper<const Ts>...>;

	template<class Tuple2>
	using cat = typename tuple::cat<std::tuple<Ts...>, Tuple2>::type;
	
	template<class T>
	using prepend = std::tuple<T, Ts...>;

	template<class Tuple>
	static constexpr auto expand(Tuple&& tup)
	{
		using decay_tuple = tuple::type_mod_for_each_t<Tuple, std::decay>;
		using helper = recursive_helper<decay_tuple, 0>;
		
		static_assert(
			helper::template each_among_types<Ts...>(),
			"source tuple contains type not used in the destination tuple"
		);
		
		return tuple::preserve_wrappers::make(
			helper::template get_or_default<Ts>(std::forward<Tuple>(tup))...
		);
	}

	template<class Tuple, class... ExtraTs>
	static constexpr auto select(const Tuple& tup, const ExtraTs&...)
	{
		using decay_tuple = tuple::type_mod_for_each_t<Tuple, std::decay>;
		using helper = recursive_helper<decay_tuple, 0>;
		
		static_assert(
			helper::template each_among_types<Ts..., ExtraTs...>(),
			"source tuple contains type not used in the destination tuple"
		);
		
		return tuple::preserve_wrappers::make(
			std::get<Ts>(tup)...
		);
	}

	template<class Tuple>
	static constexpr auto copy(const Tuple& tup)
	{
		return std::make_tuple(std::get<Ts>(tup)...);
	}
};

template<class T0, class... Ts>
struct helper<std::tuple<T0, Ts...>> : helper0<std::tuple<T0, Ts...>>
{
#if 0
	// these are wrong designed function - it shouldn't create rvalue tuple by lvalue one
	// (problems with subsequent std::get returning rvalue for the tuple elements)
	static auto tail(std::tuple<T0, Ts...>&& tup)
	{
		return std::forward_as_tuple(std::get<Ts>(std::move(tup))...);
	}
	
	static auto tail(std::tuple<T0, Ts...>& tup)
	{
		return std::forward_as_tuple(std::get<Ts>(tup)...);
	}
	
	static auto tail(const std::tuple<T0, Ts...>& tup)
	{
		return std::forward_as_tuple(std::get<Ts>(tup)...);
	}
	
	using tail_type = std::tuple<Ts...>;
#endif

	static auto zero_ref_tuple()
	{
		return std::tuple_cat(
			tuple::preserve_wrappers::make(types::zero_ref<T0>::ref_wrapper()),
			helper<std::tuple<Ts...>>::zero_ref_tuple()
		);
	}

	static void rewrite_refs(std::tuple<T0, Ts...>& tup) {}

	template<class Object0, class... Objects>
	static void rewrite_refs(std::tuple<T0, Ts...>& tup, const Object0& ref0, const Objects&... refs)
	{
		std::get<std::reference_wrapper<const Object0>>(tup) = std::cref(ref0);
		rewrite_refs(tup, refs...);
	}

	template<template<class, class...> class Fun, class... Args>
	static void for_each_no_result_forward_args(Args&&... args)
	{
		Fun<T0>()(std::forward<Args>(args)...);
		helper<std::tuple<Ts...>>::template for_each_no_result_forward_args<Fun>(std::forward<Args>(args)...);
	}
};

template<>
struct helper<std::tuple<>> : helper0<std::tuple<>>
{
	//using comparator = tuple_comparator<std::tuple<>>;

	static wrap_crefs zero_ref_tuple() { return std::make_tuple(); }

	static void rewrite_refs(std::tuple<>&) {}

	template<template<class, class...> class Fun, class... Args>
	static void for_each_no_result_forward_args(Args&&...)
	{
	}
};

template<class Container>
struct container_traits {};

template<template<class...> class ContT, class... Args>
struct container_traits<ContT<Args...>>
{
	template<class... _Args>
	using templ = ContT<_Args...>;
};

template<class Tuple>
struct cont_helper {};

template<class... Vs>
struct cont_helper<std::tuple<Vs...>>
{
	using containers_tuple = std::tuple<Vs...>;
	
	static auto ref_vector_at(containers_tuple& v_tup, std::size_t idx)
	{
		return tuple::preserve_wrappers::make(std::ref(std::get<Vs>(v_tup)[idx])...);
	}

	static auto ref_vector_at(const containers_tuple& v_tup, std::size_t idx)
	{
		return tuple::preserve_wrappers::make(std::cref(std::get<Vs>(v_tup)[idx])...);
	}

	static auto ref_vector_at(containers_tuple&& v_tup, std::size_t idx)
	{
		return tuple::preserve_wrappers::make(std::ref(std::get<Vs>(std::move(v_tup))[idx])...);
	}

	static auto cref_vector_at(const containers_tuple& v_tup, std::size_t idx)
	{
		return tuple::preserve_wrappers::make(std::cref(std::get<Vs>(v_tup)[idx])...);
	}

	using containers2val_tuple = std::tuple<types::remove_cvrefw_t<typename types::key_type<Vs>::type>...>;
	static auto val_vector_at(const containers_tuple& v_tup, std::size_t idx)
	{
		return containers2val_tuple(std::get<Vs>(v_tup)[idx]...);
	}

	using containers2ref_tuple = decltype(ref_vector_at(std::declval<containers_tuple>(), 0));
	using containers2cref_tuple = decltype(cref_vector_at(std::declval<containers_tuple>(), 0));
};

template<class... Vs>
struct cont_helper<const std::tuple<Vs...>> : cont_helper<std::tuple<Vs...>>
{};


template<class Tuple, class... Ts>
struct container_from_tuple {};

template<
	class V0,
	class... Vs,
	class... Ts
>
struct container_from_tuple<std::tuple<V0, Vs...>, Ts...>
{
	using type = typename std::conditional<
		std::is_same<V0, typename container_traits<V0>::template templ<Ts...>>::value,
		V0,
		void
	>::type;
};

template<class... Ts>
struct container_from_tuple<std::tuple<>, Ts...>
{
	using type = void;
}; 

template<std::size_t I, class Tuple, class Key, class Enable = void>
struct container_idx_from_tuple {};

template<
	std::size_t I,
	class V0,
	class... Vs,
	class Key
>
struct container_idx_from_tuple<
	I,
	std::tuple<V0, Vs...>,
	Key,
	std::enable_if_t<
		std::is_same<types::remove_cvrefw_t<typename types::key_type<V0>::type>, types::remove_cvrefw_t<Key>>::value
	>
>
{
	enum { value = I };
};

template<
	std::size_t I,
	class V0,
	class... Vs,
	class Key
>
struct container_idx_from_tuple<
	I,
	std::tuple<V0, Vs...>,
	Key,
	std::enable_if_t<
		!std::is_same<types::remove_cvrefw_t<typename types::key_type<V0>::type>, types::remove_cvrefw_t<Key>>::value
	>
>
	: container_idx_from_tuple<I + 1, std::tuple<Vs...>, Key>
{};

template<std::size_t I, class Key>
struct container_idx_from_tuple<I, std::tuple<>, Key> {};

#if 0
// Expand SrcTuple to DstTuple
// DstTuple always contain all possible members in the same order
template<class SrcTuple, class DstTuple>
struct expander
{
};

template<class Common, class... Ss, class... Ds>
struct expander<std::tuple<Common, Ss...>, std::tuple<Common, Ds...>>
{
};

// If the first member is not equal - skip from DstTuple
template<class S0, class D0, class... Ss, class.. Ds>
struct expander<std::tuple<S0, Ss...>, std::tuple<D0, Ds...>>
{
	using next = expander<std::tuple<S0, Ss...>, std::tuple<Ds...>>;
	
	auto expand(...)
	{
		return next::expand(...);
	}
};
#endif

template<class Field>
struct some_element_is_zero_ref_t
{
	template<class Tuple>
	void operator()(bool& result, const Tuple& tup) {}
};

template<class T>
struct some_element_is_zero_ref_t<T&>
{
	template<class Tuple>
	void operator()(bool& result, const Tuple& tup)
	{
		result = types::zero_ref<T>::check(std::get<T&>(tup));
	}
};

template<class T>
struct some_element_is_zero_ref_t<std::reference_wrapper<T>>
{
	template<class Tuple>
	void operator()(bool& result, const Tuple& tup)
	{
		auto pp = std::get<std::reference_wrapper<T>>(tup);
		result = types::zero_ref<std::reference_wrapper<T>>::check(pp);
	}
};

template<class... Args>
bool some_element_is_zero_ref(const std::tuple<Args...>& tup)
{
	using tuple_type = std::tuple<Args...>;
	using helper = tuple::helper<tuple_type>;

	bool result = false;
	helper::template for_each_no_result_forward_args<some_element_is_zero_ref_t>(result, tup);
	return result;
}

} // namespace tuple

