#pragma once

#include "tuple.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <initializer_list>
#include <iterator>
#include <functional>
#include <map>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <utility>

// Contains classes which implement different types of associative colletions (maps)
namespace map
{

// Object to index and index to object mapping
namespace two_way_object_indexer
{

// A pair (index, object reference) is used as a value
template<class Object, class Index>
class value_type : protected std::pair<Index, std::reference_wrapper<const Object>>
{
	using base = std::pair<Index, std::reference_wrapper<const Object>>;
	using index_value_type = Index;
	using index_marker_type = marker::type<marker::index_marker, Index>;
	
public:
	using first_type = index_marker_type;
	using second_type = const Object&;

	// the default constructor constructs the "no value"
	constexpr value_type()
		: base(types::no_value_value<index_marker_type>(), types::zero_ref<Object>::value()) {}

	explicit value_type(const first_type& m, const second_type& s)
		: base(types::strip(m), s)
	{
	}

	bool operator==(const value_type& o) const
	{
		if (is_no_value() || o.is_no_value())
			return is_no_value() && o.is_no_value();
		else 
			return base::first == o.base::first && base::second.get() == o.base::second.get();
	}
	
	first_type first() const 
	{
		return first_type{base::first};
	}

	second_type second() const
	{
		return base::second;
	}

	static value_type no_value()
	{
		return value_type{};
	}

	bool is_no_value() const
	{
		return base::first == index_marker_type{} || &base::second.get() == nullptr;
	}
};

template<
	class Object,	// a type used for objects
	class Index,	 // a type used for indexes
	class Object2Index, // object -> index convertion class
	class Index2Object
>
class type;

template<class Object, class Index, class Index2Object>
class iterator
{
	template<class _Object, class _Index, class _Object2Index, class _Index2Object>
	friend class two_way_object_indexer::type;

	using index_type = Index;
	using index_marker_type = marker::type<marker::index_marker, Index>;
	using index2object = Index2Object;
	static index_type no_index() { return index_marker_type{}; }

	const index2object* _i2o;
	index_type _idx;

	constexpr iterator(const index2object* i2o, index_type idx = no_index()) : _i2o(i2o), _idx(idx) {}

	static iterator no_value() { return iterator{types::zero_ref<index2object>::value(), no_index()}; }
	
public:
	using iterator_category = std::random_access_iterator_tag;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using value_type = two_way_object_indexer::value_type<Object, Index>;
	using reference = value_type;
	using pointer = void;

	// iterators are compatible if they point to the same collection and both are valid values
	bool is_compatible(const iterator& o) const
	{
		return &_i2o.get() == &o._i2o.get() && !is_no_value();
	}
	
	bool operator==(const iterator& o) const
	{
		return _idx == o._idx;
	}

	bool operator!=(const iterator& o) const
	{
		return !operator==(o);
	}

	iterator& operator++()
	{
		++_idx;
		return *this;
	}
 
	iterator operator++(int)
	{
		iterator copy = *this;
		operator++();
		return copy;
	}

	iterator& operator+=(difference_type k)
	{
		_idx += k;
		return *this;
	}

	iterator operator+(difference_type k)
	{
		iterator res = *this;
		return res += k;
	}

	iterator& operator--()
	{
		--_idx;
		return *this;
	}
 
	iterator operator--(int)
	{
		iterator copy = *this;
		operator--();
		return copy;
	}

	iterator& operator-=(difference_type k)
	{
		_idx -= k;
		return *this;
	}

	iterator operator-(difference_type k)
	{
		iterator res = *this;
		return res -= k;
	}

	iterator operator-(const iterator& it)
	{
		if (is_compatible(it))
			return *this -= it._idx;
		else
			return no_value();
	}

	value_type operator*() const
	{
		if (is_no_value())
			return value_type{};
		else
			return value_type(marker::index(_idx), (*_i2o)[_idx]);
	}

	value_type operator[](difference_type k) const
	{
		return *(*this + k);
	}

	bool operator<(const iterator& it) const
	{
		if (is_no_value())
		{
			if (it.is_no_value())
				return false;
			else
				return true; // no_value is lesser than everything except another no_value
		}
		else if (it.is_no_value())
			return false;
		else
			return _idx < it._idx; // NB: ignore _i2o part here
	}

	bool operator>(const iterator& it) const
	{
		if (is_no_value())
		{
			if (it.is_no_value())
				return false;
			else
				return false; // bottom is lesser than everything except another bottom
		}
		else if (it.is_no_value())
			return true;
		else
			return _idx > it._idx; // NB: ignore _i2o part here
	}

	bool operator>=(const iterator& it) const
	{
		return !(*this < it);
	}

	bool operator<=(const iterator& it) const
	{
		return !(*this > it);
	}

	bool is_no_value() const
	{
		return _i2o == nullptr;
	}
};

template<class Object, class Index, class Object2Index, class Index2Object>
iterator<Object, Index, /*Object2Index,*/ Index2Object> operator+(
	typename iterator<Object, Index, /*Object2Index,*/ Index2Object>::difference_type k,
	iterator<Object, Index, /*Object2Index,*/ Index2Object>& it
)
{
	return it + k;
}

/**
 * Maintains an indexed list of objects with ability to search a
 * object by an index and an index by a object.
 */
template<
	class Object,	// a type used for objects
	class Index,	 // a type used for indexes
	class Object2Index, // object -> index convertion class
	class Index2Object
>
class type
{
	using object2index = Object2Index;
	using index2object = Index2Object;
	
public:
	using index_type = Index;
	using iterator = two_way_object_indexer::iterator<Object, Index, /*Object2Index,*/ Index2Object>;
	
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;
	using value_type = typename iterator::value_type;
	using reference = typename iterator::reference;

	// the indexing is static, all access is const 
	using const_iterator = iterator;
	using const_reference = reference;

	// NB no pointers declared

	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	type() {}

	type(std::initializer_list<Object> objs)
	{
		for (auto it = objs.begin(); it != objs.end(); ++it)
			push_back(*it);

		// append "bottom" elements
		for (; _end_idx <= _index2object.size(); ++_end_idx)
			_index2object.push_back(value_type::second_type::_no_value);
	}

	// qllows to construct a container from different types of arguments,
	// like just string literals
	template<class Arg0, class... Args>
	type(Arg0&& obj0, Args&&... objs)
	{
		push_back_seq(std::forward<Arg0>(obj0), std::forward<Args>(objs)...);
	}

	const_reference front() const
	{
		return *begin();
	}
	
	const_reference back() const
	{
		auto it = end();
		--it;
		return *it;
	}
	
	template<class T>
	void push_back(const T& v)
	{
		assert(_end_idx >= 0);
		if ((std::size_t) _end_idx >= _index2object.max_size())
		{
			assert(false);
			return;
		}

		const Object& ref =_object2index.emplace(v, _end_idx).first->first;
		_index2object.emplace_back(ref);
		++_end_idx;
		assert((std::size_t)_end_idx <= _index2object.size());
	}

	template<class T0, class... Ts>
	void push_back_seq(T0&& v0, Ts&&... vs)
	{
		push_back(std::forward<T0>(v0));
		push_back_seq(std::forward<Ts>(vs)...);
	}

	void push_back_seq()
	{
	}

	iterator begin() const noexcept
	{
		return iterator(&_index2object, 0);
	}
	
	iterator end() const noexcept
	{
		assert(_end_idx >= 0);
		return iterator(&_index2object, _end_idx); // end_idx is used instead of size()
																						 // because for std::array() size() == max_size() always
	}
	
	const_iterator cbegin() const noexcept
	{
		return begin();
	}
	
	const_iterator cend() const noexcept
	{
		return end();
	}

	reverse_iterator rbegin() const noexcept
	{
		return std::reverse_iterator<iterator>(end());
	}
	
	reverse_iterator rend() const noexcept
	{
		return std::reverse_iterator<iterator>(begin());
	}
	
	const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}
	
	const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	bool operator==(const type& o) const noexcept
	{
		return std::equal(begin(), end(), o.begin(), o.end());
	}

	bool operator!=(const type& o) const noexcept
	{
		return !(*this == o);
	}

	void swap(type& o)
	{
		using std::swap;

		swap(_object2index, o._object2index);
		swap(_index2object, o._index2object);
		swap(_end_idx, o._end_idx);
	}

	size_type size() const noexcept
	{
		assert(_end_idx >= 0);
		return _end_idx;
	}

	constexpr size_type max_size() const
	{
		return _index2object.max_size();
	}

	size_type empty() const noexcept
	{
		return size() == 0;
	}

	iterator find(typename value_type::first_type idx_marker) const
	{
		typename value_type::first_type::value_type idx_int;
		if (!idx_marker.in_range(_index2object, idx_int))
			return end();

		return begin() + idx_int;
	}
	
	iterator find(const Object& obj) const
	{
		const auto it = _object2index.find(obj);
		if (it == _object2index.end())
			return end();

		return begin() + it->second;
	}

	reference operator[](typename value_type::first_type idx) const
	{
		auto it = find(idx);
		if (it < begin() || it >= end())
			return value_type{};
		else
			return *it;
	}
	
	
	reference operator[](const Object& obj) const
	{
		auto it = find(obj);
		if (it < begin() || it >= end())
			return value_type{};
		else
			return *it;
	}
	
	
	reference at(typename value_type::first_type idx) const
	{
		auto it = find(idx);
		if (it < begin() || it >= end())
			throw std::out_of_range("two_way_object_indexer at()");
		else
			return *it;
	}
	
	reference at(const Object& obj) const
	{
		auto it = find(obj);
		if (it < begin() || it >= end())
			throw std::out_of_range("two_way_object_indexer at()");
		else
			return *it;
	}

	reference get_or_insert(const Object& obj)
	{
		auto it = find(obj);
		if (it == end())
			{
				push_back(obj);
				it = end() - 1;
			}
		assert((*it).second == obj);
		return *it;
	}
	
private:
	object2index _object2index;
	index2object _index2object;
	index_type _end_idx = 0;
};

template<class O, class I, class O2I, class I2O>
void swap(type<O, I, O2I, I2O>& a, type<O, I, O2I, I2O>& b)
{
	a.swap(b);
}

} // namespace two_way_object_indexer

namespace multi_way_object_indexer
{


/* the indexed multi-value container value_type */
template<class Index, class ValTuple>
class value_type
	: public tuple::helper<ValTuple>::template prepend<marker::type<marker::index_marker, Index>>
{
	template<class _Index, class _ValTuple>
	friend std::ostream& operator<<(std::ostream& out, const value_type<_Index, _ValTuple>& val);
	
	using index_value_type = Index;
public:
	using base = typename tuple::helper<ValTuple>::template prepend<marker::type<marker::index_marker, Index>>;

	using index_marker_type = marker::type<marker::index_marker, Index>;
	using value_tuple = ValTuple;
	
	// the default constructor constructs the "no value"
	constexpr value_type() : value_type(index_marker_type{}) {}

	template<class... Ts>
	value_type(std::tuple<Ts...>&& t)
		: base(std::tuple_cat(std::make_tuple(index_marker_type{}), make_value_tuple(std::move(t))))
	{}
	
	explicit value_type(const index_marker_type& idx)
		: base(std::tuple_cat(std::make_tuple(idx), value_tuple()))
	{}

	value_type(const index_marker_type& idx, const ValTuple& tup)
		: base(std::tuple_cat(std::make_tuple(idx), tup))
	{}

	value_type(const index_marker_type& idx, ValTuple&& tup)
		: base(std::tuple_cat(std::make_tuple(idx), std::move(tup)))
	{}

	// a value tuple doesn't contain index (and has no no_type).
	template<class... Ts> 
	static value_tuple make_value_tuple(std::tuple<Ts...>&& tup)
	{
		return tuple::helper<value_tuple>::expand(std::move(tup));
	}

	value_tuple get_value_tuple() const
	{
		return tuple::helper<value_tuple>::select(
			static_cast<const base&>(*this),
			index_marker_type{}
		);
	}
	
	bool operator==(const value_type& o) const
	{
		if (is_no_value() || o.is_no_value())
			return is_no_value() && o.is_no_value();
		else 
			return base::operator==(o);
	}

	index_marker_type index() const 
	{
		return index_marker_type{std::get<0>(*this)};
	}
	
	static value_type no_value()
	{
		return value_type{};
	}

	bool is_no_value() const
	{
		return index() == index_marker_type{}; /* || &base::second.get() == nullptr;*/
	}
};

template<class T>
struct select_functor
{
	template<class DstTuple, class SrcTuple>
	void operator()(DstTuple& dst, const SrcTuple& src) const
	{
		using helper = tuple::recursive_helper<SrcTuple, 0>;
		std::get<T>(dst) = helper::template get_by_unref_type<T>(src);
	}
};

template<class T>
struct push_functor_2w
{
	template<
		class Index,
		class object2index_tuple,
		class index2object_tuple,
		class value_tuple
	>
	void operator()(
		object2index_tuple& o2i_tup,
		index2object_tuple& i2o_tup,
		value_tuple&& tup,
		Index idx
	) const
	{
		auto& o2i = std::get<
			tuple::container_idx_from_tuple<0, object2index_tuple, T>::value
		>(o2i_tup);
		
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<
				0,
				index2object_tuple,
				std::reference_wrapper<const T>>::value
		>(i2o_tup);
				
		i2o.emplace_back(std::get<T>(std::forward<value_tuple>(tup)));
		assert((std::size_t)idx == i2o.size() - 1);

		o2i.emplace(i2o.back(), idx);
	  // NB: ignore the result of this insertion		
	}
};

template<class T>
struct push_functor_1w
{
	template<
		class Index,
		class index2object_tuple,
		class value_tuple
	>
	void operator()(
		index2object_tuple& i2o_tup,
		value_tuple&& tup,
		Index idx
	) const
	{
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<0, index2object_tuple, T>::value
		>(i2o_tup);

		i2o.emplace_back(std::get<T>(std::forward<value_tuple>(tup)));
		assert((std::size_t)idx == i2o.size() - 1);
	}
};

template<class T>
struct rewrite_functor_2w
{
	template<
		class Index,
		class object2index_tuple,
		class index2object_tuple,
		class value_tuple
	>
	void operator()(
		object2index_tuple& o2i_tup,
		index2object_tuple& i2o_tup,
		value_tuple&& tup,
		Index idx
	) const
	{
		auto& o2i = std::get<
			tuple::container_idx_from_tuple<0, object2index_tuple, T>::value
		>(o2i_tup);
		
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<
				0,
				index2object_tuple,
				std::reference_wrapper<const T>>::value
		>(i2o_tup);
				
		auto& ref = i2o.at(idx);
		ref = std::get<T>(std::forward<value_tuple>(tup));

		o2i.emplace(ref, idx);
	  // NB: ignore the result of this insertion		
	}
};

#if 0 // the proper implementation is under protect<> below
template<class T>
struct rewrite_functor_1w
{
	template<
		class Index,
		class index2object_tuple,
		class value_tuple
	>
	void operator()(
		index2object_tuple& i2o_tup,
		value_tuple&& tup,
		Index idx
	) const
	{
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<0, index2object_tuple, T>::value
		>(i2o_tup);

		i2o.at(idx) = std::get<T>(std::forward<value_tuple>(tup));
	}
};
#endif

template<class TwoWayObjects>
struct protect // protect from imroper instantiation
{

template<class T>
struct rewrite_functor_1w
{
	static_assert(
		!tuple::among_tuple_types<types::remove_cvref_t<T>, TwoWayObjects>::value,
		"don't partially update 2 way-objects - the full row must be updated in this case"
	);
		
	template<
		class Index,
		class index2object_tuple,
		class value_tuple
	>
	void operator()(index2object_tuple& i2o_tup, value_tuple&& tup, Index idx) const
	{
		typename Index::value_type idx_int;
		if (!types::get_value(idx, idx_int))
			return;
		
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<0, index2object_tuple, types::remove_cvref_t<T>>::value
		>(i2o_tup);

		i2o.at(idx_int) = tuple::recursive_helper<value_tuple, 0>::template get_by_unref_type<T>(std::forward<value_tuple>(tup));
	}
};

};

template<class T>
struct erase_functor_2w
{
	template<
		class Index,
		class object2index_tuple,
		class index2object_tuple
	>
	void operator()(object2index_tuple& o2i_tup, index2object_tuple& i2o_tup, Index idx) const
	{
		auto& o2i = std::get<tuple::container_idx_from_tuple<0, object2index_tuple, T>::value>(o2i_tup);
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<0, index2object_tuple, std::reference_wrapper<const T>>::value
		>(i2o_tup);

		const auto& obj = i2o.at(idx);

		const auto range = o2i.equal_range(obj);
		for (auto it = range.first; it != range.second; ++it)
		{
			if (&it->first.get() == &obj)
			{
				o2i.erase(it);
				break;
			}
		}
		i2o[idx] = T();
	}
};

template<class T>
struct erase_functor_1w
{
	template<
		class Index,
		class index2object_tuple
	>
	void operator()(index2object_tuple& i2o_tup, Index idx) const
	{
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<0, index2object_tuple, std::reference_wrapper<const T>>::value
		>(i2o_tup);

		i2o.at(idx) = T();
	}
};

template<class T>
struct update_keys_functor_2w
{
	template<
		class Index,
		class object2index_tuple,
		class index2object_tuple
	>
	void operator()(
		object2index_tuple& o2i_tup,
		index2object_tuple& i2o_tup,
		Index old_idx,
		Index new_idx
	) const
	{
		auto& o2i = std::get<tuple::container_idx_from_tuple<0, object2index_tuple, T>::value>(o2i_tup);
		auto& i2o = std::get<
			tuple::container_idx_from_tuple<0, index2object_tuple, std::reference_wrapper<const T>>::value
		>(i2o_tup);

		const auto& old_key = i2o.at(old_idx);
		const auto& new_key = i2o.at(new_idx);

		const auto old_it = o2i.find(old_key);
		assert(old_it != o2i.end());

		if (old_key == new_key)
			o2i.emplace_hint(old_it, new_key, new_idx); // NB use the hint
		else
			o2i.emplace(new_key, new_idx);

		// NB remove the old key only after the hint was used
		o2i.erase(old_it);
	}
};

namespace thread_safe
{

template<
	template<class, class> class Object2IndexT,
	template<class> class Index2ObjectT,
	class Index,
	class TwoWayObjects,
	class OneWayObjects,
	class Mutex
>
class type;

template<class Index, class RefTuple, class ValTuple>
class reference_type
	: protected tuple::helper<RefTuple>::template prepend<marker::type<marker::index_marker, Index>>
{
	template<
		template<class, class> class _Object2IndexT,
		template<class> class _Index2ObjectT,
		class _Index,
		class _TwoWayObjects,
		class _OneWayObjects,
		class _Mutex
	>
	friend class multi_way_object_indexer::thread_safe::type;

	using base = typename tuple::helper<RefTuple>::template prepend<marker::type<marker::index_marker, Index>>;

	template<class _Index, class _RefTuple, class _ValTuple>
	friend std::ostream& operator<<(std::ostream& out, const reference_type<_Index, _RefTuple, _ValTuple>& val);
	
	using index_value_type = Index;

public:
	reference_type() : reference_type(index_marker_type{}, tuple::helper<RefTuple>::zero_ref_tuple()) {}
	
	reference_type(const reference_type&) = delete;

	reference_type(reference_type&&) = default;
	
	using index_marker_type = marker::type<marker::index_marker, Index>;
	
	explicit reference_type(const index_marker_type& idx, const RefTuple& tup)
		: base(std::tuple_cat(std::make_tuple(idx), tup))
	{}

	reference_type& operator=(const reference_type&) = delete;
	
	reference_type& operator=(reference_type&&) = default;
	
	operator value_type<Index, ValTuple>() const
	{
		using v_type = value_type<Index, ValTuple>;
		
		return v_type(this->index(), tuple::helper<typename v_type::base>::copy(*this));
	}
	
	bool operator==(const reference_type& o) const
	{
		if (this->is_no_value() || o.is_no_value())
			return this->is_no_value() && o.is_no_value();
		else 
			return tuple::helper<base>::comparator::equal(*this, o);
	}

	// TODO left val, right ref
	bool operator==(const value_type<Index, ValTuple>& o) const
	{
		using val_type = value_type<Index, ValTuple>;
		
		if (this->is_no_value() || o.is_no_value())
			return this->is_no_value() && o.is_no_value();
		else 
			return tuple::comparator<base, typename val_type::base, 0>::equal(static_cast<const base& >(*this), static_cast<const typename val_type::base& >(o));
	}

	index_marker_type index() const 
	{
		return index_marker_type{std::get<0>(*this)};
	}

	template<
		class T,
		typename std::enable_if<!std::is_const<T>::value, bool>::type = false
	>
	T& by_type()
	{
		return tuple::recursive_helper<base, 0>::template get_by_unref_type<T>(static_cast<base&>(*this));
	}

	template<
		class T
	>
	const T& by_type() const
	{
		return tuple::recursive_helper<base, 0>::template get_by_unref_type<T>(static_cast<const base&>(*this));
	}

	static reference_type no_value()
	{
		return reference_type{};
	}

	bool is_no_value() const
	{
		return index() == index_marker_type{}; /* || &base::second.get() == nullptr;*/
	}
};

template<class Index, class ValTuple>
std::ostream& operator<<(std::ostream& out, const value_type<Index, ValTuple>& val)
{
	using val_type = value_type<Index, ValTuple>;
	using tuple_type = typename val_type::base;
	
	out << "value_type{";
	tuple::recursive_helper<tuple_type, 0>::out(out, (const tuple_type&)val);
	out << '}';
	return out;
}

template<class Index, class RefTuple, class ValTuple>
std::ostream& operator<<(std::ostream& out, const reference_type<Index, RefTuple, ValTuple>& val)
{
	using ref_type = reference_type<Index, RefTuple, ValTuple>;
	using tuple_type = typename ref_type::base;
	
	out << "value_type{";
	tuple::recursive_helper<tuple_type, 0>::out(out, (const tuple_type&)val);
	out << '}';
	return out;
}

template<class Index, class Index2ObjectsTuple, class Mutex>
class iterator
{
	template<
		template<class, class> class _Object2IndexT,
		template<class> class _Index2ObjectT,
		class _Index,
		class _TwoWayObjects,
		class _OneWayObjects,
		class _Mutex
	>
	friend class multi_way_object_indexer::thread_safe::type;

	using index_type = Index;
	using index_marker_type = marker::type<marker::index_marker, Index>;
	using index2object = Index2ObjectsTuple;
	using unique_lock = std::unique_lock<Mutex>;

	index2object* _i2o;
	index_type _idx;
	std::reference_wrapper<const unique_lock> _lock;
	
	constexpr iterator(
		const unique_lock& lock,
		index2object* i2o,
		index_type idx = types::no_value_value<index_marker_type>()
	)
		: _i2o(i2o), _idx(idx), _lock(lock)
	{}

	static iterator no_value()
	{
		return types::zero_ref<index2object>::value();
	};
	
public:
	using iterator_category = std::random_access_iterator_tag;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using cont_tuple_helper = tuple::cont_helper<Index2ObjectsTuple>;
	using ref_tuple = typename cont_tuple_helper::containers2ref_tuple;
	using cref_tuple = typename cont_tuple_helper::containers2cref_tuple;
	using val_tuple = typename cont_tuple_helper::containers2val_tuple;
	using value_type = multi_way_object_indexer::value_type<Index, val_tuple>;
	using reference = multi_way_object_indexer::thread_safe::reference_type<Index, ref_tuple, val_tuple>;
	using const_reference = multi_way_object_indexer::thread_safe::reference_type<Index, cref_tuple, val_tuple>;
	using pointer = void;

	~iterator()
	{
		assert(_lock.get().owns_lock()); // the iterator should be destroyed before the lock
	}
	
	// iterators are compatible if they point to the same collection and both are valid values
	bool is_compatible(const iterator& o) const
	{
		return &_i2o.get() == &o._i2o.get() && !is_no_value();
	}
	
	bool operator==(const iterator& o) const
	{
		return _idx == o._idx;
	}

	bool operator!=(const iterator& o) const
	{
		return !operator==(o);
	}

	iterator& operator++()
	{
		++_idx;
	}
 
	iterator operator++(int)
	{
		iterator copy = *this;
		operator++();
		return copy;
	}

	iterator& operator+=(difference_type k)
	{
		_idx += k;
		return *this;
	}

	iterator operator+(difference_type k)
	{
		iterator res = *this;
		return res += k;
	}

	iterator& operator--()
	{
		--_idx;
		return *this;
	}
 
	iterator operator--(int)
	{
		iterator copy = *this;
		operator--();
		return copy;
	}

	iterator& operator-=(difference_type k)
	{
		_idx -= k;
		return *this;
	}

	iterator operator-(difference_type k)
	{
		iterator res = *this;
		return res -= k;
	}

	iterator operator-(const iterator& it)
	{
		if (is_compatible(it))
			return *this -= it._idx;
		else
			return no_value();
	}

	reference operator*()
	{
		if (is_no_value())
			return reference{};
		else
			return reference(marker::index(_idx), cont_tuple_helper::ref_vector_at(*_i2o, _idx));
	}

	const_reference operator*() const
	{
		if (is_no_value())
			return reference{};
		else
			return const_reference(marker::index(_idx), cont_tuple_helper::cref_vector_at(*_i2o, _idx));
	}

	reference operator[](difference_type k)
	{
		return *(*this + k);
	}

	const_reference operator[](difference_type k) const
	{
		return *(*this + k);
	}

	bool operator<(const iterator& it) const
	{
		if (is_no_value())
		{
			if (it.is_no_value())
				return false;
			else
				return true; // no_value is lesser than everything except another no_value
		}
		else if (it.is_no_value())
			return false;
		else
			return _idx < it._idx; // NB: ignore _i2o part here
	}

	bool operator>(const iterator& it) const
	{
		if (is_no_value())
		{
			if (it.is_no_value())
				return false;
			else
				return false; // bottom is lesser than everything except another bottom
		}
		else if (it.is_no_value())
			return true;
		else
			return _idx > it._idx; // NB: ignore _i2o part here
	}

	bool operator>=(const iterator& it) const
	{
		return !(*this < it);
	}

	bool operator<=(const iterator& it) const
	{
		return !(*this > it);
	}

	bool is_no_value() const
	{
		return types::has_no_value(index_marker_type{_idx});
	}
};

/**
 * Maintains an indexed list of objects with ability to search a
 * object by an index and an index by a object.
 */
template<
	template<class, class> class Object2IndexT,
	template<class> class Index2ObjectT,
	class Index,
	class TwoWayObjects,
	class OneWayObjects,
	class Mutex = std::mutex
>
class type
{
	using all_objects_tuple = typename tuple::helper<TwoWayObjects>::template cat<OneWayObjects>;
	using tuple_helper_all = tuple::helper<all_objects_tuple>;
	using tuple_helper_1w = tuple::helper<OneWayObjects>;
	using tuple_helper_2w = tuple::helper<TwoWayObjects>;

	// 2w objects are RO, 1w - RW
	using index2objects_tuple = typename tuple::helper<typename tuple_helper_2w::template tuple_of_containers<Index2ObjectT>>::template cat<
		typename tuple_helper_1w::template tuple_of_containers<Index2ObjectT>
	>;
	
	using objects2index_tuple = typename tuple_helper_2w::template tuple_of_cref_maps<Object2IndexT, Index>;
public:
	using index_type = Index;
	using index_marker_type = marker::type<marker::index_marker, Index>;
	using iterator = multi_way_object_indexer::thread_safe::iterator<Index, index2objects_tuple, Mutex>;
	using const_iterator = multi_way_object_indexer::thread_safe::iterator<Index, const index2objects_tuple, Mutex>;
	
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;
	using value_type = typename iterator::value_type;
	using value_tuple = typename value_type::value_tuple;
	using reference = typename iterator::reference;
	using const_reference = typename iterator::const_reference;
	
	using mutex_type = Mutex;
	using lock_guard = std::lock_guard<mutex_type>;
	using unique_lock = std::unique_lock<mutex_type>;
	
	// NB no pointers declared

	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	mutable mutex_type _mutex;
	
	type() {}

	template<class... Ts>
	type(std::initializer_list<std::tuple<Ts...>> objs)
	{
		for (auto it = objs.begin(); it != objs.end(); ++it)
			push_back(*it);
	}

	// allows to construct a container from different types of arguments
	template<class Arg0, class... Args>
	type(Arg0&& obj0, Args&&... objs)
	{
		push_back_seq(std::forward<Arg0>(obj0), std::forward<Args>(objs)...);
	}

	reference front()
	{
		unique_lock lock(_mutex);
		
		return *begin(lock);
	}
	
	const_reference front() const
	{
		unique_lock lock(_mutex);
		
		return *begin(lock);
	}
	
	reference back()
	{
		unique_lock lock(_mutex);
		
		auto it = end(lock);
		--it;
		return *it;
	}

	const_reference back() const
	{
		unique_lock lock(_mutex);
		
		auto it = end(lock);
		--it;
		return *it;
	}

#if 0
	// the version compatible with std::back_insert_iterator
	void push_back(value_type&& v)
	{
		lock_guard lock(_mutex);

		push_back_int(std::move(v));
	}
#else
	template<class T>
	void push_back(T&& v)
	{
		lock_guard lock(_mutex);

		push_back_int(value_type::make_value_tuple(std::move(v)));
	}
#endif

	template<class T0, class... Ts>
	void push_back_seq(T0&& v0, Ts&&... vs)
	{
		push_back(std::forward<T0>(v0));
		push_back_seq(std::forward<Ts>(vs)...);
	}

	void push_back_seq()
	{
	}

	iterator begin(const unique_lock& lock) noexcept
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return iterator(lock, &_index2object_tuple);
		}
		
		return iterator(lock, &_index2object_tuple, 0);
	}
	
	const_iterator begin(const unique_lock& lock) const noexcept
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return const_iterator(lock, &_index2object_tuple);
		}
		
		return const_iterator(lock, &_index2object_tuple, 0);
	}
	
	iterator end(const unique_lock& lock) noexcept
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return iterator(lock, &_index2object_tuple);
		}
		
		assert(_end_idx >= 0);
		return iterator(lock, &_index2object_tuple, _end_idx);
	}
	
	const_iterator end(const unique_lock& lock) const noexcept
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return const_iterator(lock, &_index2object_tuple);
		}
		
		assert(_end_idx >= 0);
		return const_iterator(lock, &_index2object_tuple, _end_idx);
	}
	
	const_iterator cbegin(const unique_lock& lock) const noexcept
	{
		return begin(lock);
	}
	
	const_iterator cend(const unique_lock& lock) const noexcept
	{
		return end(lock);
	}

	reverse_iterator rbegin(const unique_lock& lock) noexcept
	{
		return std::reverse_iterator<iterator>(end(lock));
	}
	
	reverse_iterator rend(const unique_lock& lock) noexcept
	{
		return std::reverse_iterator<iterator>(begin(lock));
	}
	
	const_reverse_iterator crbegin(const unique_lock& lock) const noexcept
	{
		return std::reverse_iterator<const_iterator>(end(lock));
	}
	
	const_reverse_iterator crend(const unique_lock& lock) const noexcept
	{
		return std::reverse_iterator<const_iterator>(begin(lock));
	}

	bool operator==(const type& o) const noexcept
	{
		std::lock(_mutex, o._mutex);
		unique_lock lock(_mutex, std::adopt_lock);
		unique_lock o_lock(o._mutex, std::adopt_lock);
		
		return std::equal(begin(lock), end(lock), o.begin(o_lock), o.end(o_lock));
	}

	bool operator!=(const type& o) const noexcept
	{
		return !(*this == o);
	}

	void swap(type& o)
	{
		using std::swap;

		std::lock(_mutex, o._mutex);
		unique_lock lock(_mutex, std::adopt_lock);
		unique_lock o_lock(o._mutex, std::adopt_lock);

		swap(_object2index_tuple, o._object2index_tuple);
		swap(_index2object_tuple, o._index2object_tuple);
		swap(_end_idx, o._end_idx);
	}

	size_type size() const noexcept
	{
		assert(_end_idx >= 0);
		return _end_idx;
	}

	size_type empty() const noexcept
	{
		return size() == 0;
	}

	iterator find(index_marker_type idx_marker, const unique_lock& lock)
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return iterator(lock, &_index2object_tuple);
		}
		
		typename index_marker_type::value_type idx;
		if (!types::get_value(idx_marker, idx))
			return end(lock);
		
		if (idx >= std::get<0>(_index2object_tuple).size())
			return end(lock);

		return begin(lock) + idx;
	}

	const_iterator find(index_marker_type idx_marker, const unique_lock& lock) const
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return const_iterator(lock, &_index2object_tuple);
		}
		
		auto idx = idx_marker._value;
		
		if (idx >= std::get<0>(_index2object_tuple).size() || idx < 0)
			return end(lock);

		return begin(lock) + idx;
	}

	template<class T>
	iterator find(const T& obj, const unique_lock& lock)
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return iterator(lock, &_index2object_tuple);
		}

		auto& o2i = object2index<T>();
		
		const auto it = o2i.find(obj);
		if (it == o2i.end())
			return end(lock);

		return begin(lock) + it->second;
	}

	template<class T>
	const_iterator find(const T& obj, const unique_lock& lock) const
	{
		if (lock.mutex() != &_mutex || !lock.owns_lock())
		{
			assert(false);
			return const_iterator(lock, &_index2object_tuple);
		}

		auto& o2i = object2index<T>();
		
		const auto it = o2i.find(obj);
		if (it == o2i.end())
			return end(lock);

		return begin(lock) + it->second;
	}

	template<class T>
	reference operator[](const T& obj)
	{
		unique_lock lock(_mutex);
		
		auto it = find(obj, lock);
		if (it < begin(lock) || it >= end(lock))
			return reference{};
		else
			return *it;
	}
	
	template<class T>
	const_reference operator[](const T& obj) const
	{
		unique_lock lock(_mutex);
		
		auto it = find(obj, lock);
		if (it < begin(lock) || it >= end(lock))
			return const_reference{};
		else
			return *it;
	}
	
	
	template<class T>
	reference at(const T& obj)
	{
		unique_lock lock(_mutex);
		
		auto it = find(obj, lock);
		if (it < begin(lock) || it >= end(lock))
			throw std::out_of_range("two_way_object_indexer at()");
		else
			return *it;
	}

	template<class T>
	const_reference at(const T& obj) const
	{
		unique_lock lock(_mutex);
		
		auto it = find(obj, lock);
		if (it < begin(lock) || it >= end(lock))
			throw std::out_of_range("two_way_object_indexer at()");
		else
			return *it;
	}

	template<class Key, class... Pars>
	std::pair<reference, bool> update_or_insert(const Key& key, Pars&&... pars)
	{
		unique_lock lock(_mutex);
		bool insert = false;
		
		auto it = find(key, lock);
		if (it == end(lock))
		{
			push_back_int(value_type::make_value_tuple(std::forward_as_tuple(key, std::forward<Pars>(pars)...)));
			it = end(lock) - 1;
			insert = true;
		}
		else
		{
			tuple::helper<std::tuple<Pars...>>::template for_each_no_result_forward_args<protect<TwoWayObjects>::template rewrite_functor_1w>(
				_index2object_tuple,
				std::forward_as_tuple(std::forward<Pars>(pars)...),
				(*it).template by_type<const index_marker_type>()
			);
		}
		assert((*it).template by_type<const Key>() == key);
		return std::make_pair(*it, insert);
	}

protected:
	void push_back_int(value_tuple&& tup)
	{
		// 2 way objects
		
		tuple_helper_2w::template for_each_no_result_forward_args<push_functor_2w>(
			_object2index_tuple,
			_index2object_tuple,
			std::move(tup),
			_end_idx
		);

		// 1 way objects

		tuple_helper_1w::template for_each_no_result_forward_args<push_functor_1w>(
			_index2object_tuple,
			std::move(tup),
			_end_idx
		);

		++_end_idx;
	}
	
	template<class T>
	Object2IndexT<std::reference_wrapper<const T>, Index>& object2index()
	{
		static_assert(
			tuple::among_tuple_types<T, TwoWayObjects>::value,
			"Unable to index the collection by the type not specified in TwoWayObjects"
		);
		return std::get<Object2IndexT<std::reference_wrapper<const T>, Index>>(_object2index_tuple);
	}

	template<class T>
	const Object2IndexT<std::reference_wrapper<const T>, Index>& object2index() const
	{
		static_assert(
			tuple::among_tuple_types<T, TwoWayObjects>::value,
			"Unable to index the collection by the type not specified in TwoWayObjects"
		);
		return std::get<Object2IndexT<std::reference_wrapper<const T>, Index>>(_object2index_tuple);
	}

	template<class T>
	Index2ObjectT<T>& index2object()
	{
		return std::get<Index2ObjectT<T>>(_index2object_tuple);
	}

	template<class T>
	const Index2ObjectT<T>& index2object() const
	{
		return std::get<Index2ObjectT<T>>(_index2object_tuple);
	}

private:
	objects2index_tuple _object2index_tuple;
	index2objects_tuple _index2object_tuple;
	index_type _end_idx = 0;
};

} // namespace thread_safe
 
template<
	template<class, class> class Object2IndexT,
	template<class> class Index2ObjectT,
	class Index,
	class TwoWayObjects,
	class OneWayObjects
>
class type;

template<class Index, class RefTuple, class ValTuple>
class reference_type
#if 0
	: protected
#else
  : public
#endif
tuple::helper<RefTuple>::template prepend<marker::type<marker::index_marker, Index>>
{
	template<
		template<class, class> class _Object2IndexT,
		template<class> class _Index2ObjectT,
		class _Index,
		class _TwoWayObjects,
		class _OneWayObjects
	>
	friend class multi_way_object_indexer::type;

	using base = typename tuple::helper<RefTuple>::template prepend<marker::type<marker::index_marker, Index>>;

	template<class _Index, class _RefTuple, class _ValTuple>
	friend std::ostream& operator<<(std::ostream& out, const reference_type<_Index, _RefTuple, _ValTuple>& val);
	
	using index_value_type = Index;

public:
	reference_type() : reference_type(index_marker_type{}, tuple::helper<RefTuple>::zero_ref_tuple()) {}

	reference_type(const reference_type&) = delete;

	reference_type(reference_type&&) = default;
	
	using index_marker_type = marker::type<marker::index_marker, Index>;
	
	explicit reference_type(const index_marker_type& idx, const RefTuple& tup)
		: base(std::tuple_cat(std::make_tuple(idx), tup))
	{}

	reference_type& operator=(const reference_type&) = delete;
	
	reference_type& operator=(reference_type&&) = default;

	template<class... Ts>
	reference_type& operator=(std::tuple<Ts...>&& t)
	{
		int a = t;
	}
	
	template<class... Ts>
	reference_type& operator|=(std::tuple<Ts...>&& t)
	{
		int a = t;
	}
	
	explicit operator value_type<Index, ValTuple>() const
	{
 		using v_type = value_type<Index, ValTuple>;
		using helper = tuple::helper<ValTuple>;

		//ValTuple result;
		v_type result;

		helper::template for_each_no_result_forward_args<select_functor>(
			result,
			static_cast<const base&>(*this)
		);
		return result; //v_type(index(), std::move(result));
	}

	value_type<Index, ValTuple> value() const
	{
		return (value_type<Index, ValTuple>)*this;
	}
	
	bool operator==(const reference_type& o) const
	{
		if (this->is_no_value() || o.is_no_value())
			return this->is_no_value() && o.is_no_value();
		else 
			return tuple::helper<base>::comparator::equal(*this, o);
	}

	// TODO left val, right ref
	bool operator==(const value_type<Index, ValTuple>& o) const
	{
		using val_type = value_type<Index, ValTuple>;
		
		if (this->is_no_value() || o.is_no_value())
			return this->is_no_value() && o.is_no_value();
		else 
			return tuple::comparator<base, typename val_type::base, 0>::equal(static_cast<const base& >(*this), static_cast<const typename val_type::base& >(o));
	}

	index_marker_type index() const 
	{
		return index_marker_type{std::get<0>(*this)};
	}

	template<
		class T,
		typename std::enable_if<!std::is_const<T>::value, bool>::type = false
	>
	T& by_type()
	{
		if (!is_no_value())
			return tuple::recursive_helper<base, 0>::template get_by_unref_type<T>(static_cast<base&>(*this));
		else
		{
			assert(false);
			static T no_value{types::no_value<T>()};
			return no_value;
		}
	}

	template<
		class T
	>
	const T& by_type() const
	{
		if (!is_no_value())
			return tuple::recursive_helper<base, 0>::template get_by_unref_type<T>(static_cast<const base&>(*this));
		else
		{
			//assert(false);
			static T no_value{types::no_value<T>()};
			return no_value;
		}
	}

	static reference_type no_value()
	{
		return reference_type{};
	}

	bool is_no_value() const
	{
		return index() == index_marker_type{} || tuple::some_element_is_zero_ref(*this);
	}
	
	bool is_no_value_strong() const
	{
		return is_no_value() || types::has_no_value(value().get_value_tuple());
	}
};

template<class Index, class ValTuple>
std::ostream& operator<<(std::ostream& out, const value_type<Index, ValTuple>& val)
{
	using val_type = value_type<Index, ValTuple>;
	using tuple_type = typename val_type::base;
	
	out << "value_type{";
	tuple::recursive_helper<tuple_type, 0>::out(out, (const tuple_type&)val);
	out << '}';
	return out;
}

template<class Index, class RefTuple, class ValTuple>
std::ostream& operator<<(std::ostream& out, const reference_type<Index, RefTuple, ValTuple>& val)
{
	using ref_type = reference_type<Index, RefTuple, ValTuple>;
	using tuple_type = typename ref_type::base;
	
	out << "value_type{";
	tuple::recursive_helper<tuple_type, 0>::out(out, (const tuple_type&)val);
	out << '}';
	return out;
}

template<class Index, class Index2ObjectsTuple>
class iterator
{
	template<
		template<class, class> class _Object2IndexT,
		template<class> class _Index2ObjectT,
		class _Index,
		class _TwoWayObjects,
		class _OneWayObjects
	>
	friend class multi_way_object_indexer::type;

	using index_type = Index;
	using index_marker_type = marker::type<marker::index_marker, Index>;
	using index2object = Index2ObjectsTuple;

	index2object* _i2o = nullptr;
	index_type _idx;
	
	constexpr iterator(
		index2object* i2o,
		index_type idx = index_marker_type::no_value()._value
	)
		: _i2o(i2o), _idx(idx)
	{}

	static iterator no_value()
	{
		return types::zero_ref<index2object>::value();
	};
	
public:
	using iterator_category = std::random_access_iterator_tag;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using cont_tuple_helper = tuple::cont_helper<Index2ObjectsTuple>;
	using ref_tuple = typename cont_tuple_helper::containers2ref_tuple;
	using cref_tuple = typename cont_tuple_helper::containers2cref_tuple;
	using val_tuple = typename cont_tuple_helper::containers2val_tuple;
	using value_type = multi_way_object_indexer::value_type<Index, val_tuple>;
	using reference = multi_way_object_indexer::reference_type<Index, ref_tuple, val_tuple>;
	using const_reference = multi_way_object_indexer::reference_type<Index, cref_tuple, val_tuple>;
	using pointer = void;

	// iterators are compatible if they point to the same collection and both are valid values
	bool is_compatible(const iterator& o) const
	{
		return &_i2o.get() == &o._i2o.get() && !is_no_value();
	}
	
	bool operator==(const iterator& o) const
	{
		return _idx == o._idx;
	}

	bool operator!=(const iterator& o) const
	{
		return !operator==(o);
	}

	iterator& operator++()
	{
		++_idx;
		return *this;
	}
 
	iterator operator++(int)
	{
		iterator copy = *this;
		operator++();
		return copy;
	}

	iterator& operator+=(difference_type k)
	{
		_idx += k;
		return *this;
	}

	iterator operator+(difference_type k)
	{
		iterator res = *this;
		return res += k;
	}

	iterator& operator--()
	{
		--_idx;
		return *this;
	}
 
	iterator operator--(int)
	{
		iterator copy = *this;
		operator--();
		return copy;
	}

	iterator& operator-=(difference_type k)
	{
		_idx -= k;
		return *this;
	}

	iterator operator-(difference_type k)
	{
		iterator res = *this;
		return res -= k;
	}

	iterator operator-(const iterator& it)
	{
		if (is_compatible(it))
			return *this -= it._idx;
		else
			return no_value();
	}

	reference operator*()
	{
		if (is_no_value())
			return reference{};
		else
			return reference(marker::index(_idx), cont_tuple_helper::ref_vector_at(*_i2o, _idx));
	}

	const_reference operator*() const
	{
		if (is_no_value())
			return const_reference{};
		else
			return const_reference(marker::index(_idx), cont_tuple_helper::cref_vector_at(*_i2o, _idx));
	}

	reference operator[](difference_type k)
	{
		return *(*this + k);
	}

	const_reference operator[](difference_type k) const
	{
		return *(*this + k);
	}

	bool operator<(const iterator& it) const
	{
		if (is_no_value())
		{
			if (it.is_no_value())
				return false;
			else
				return true; // no_value is lesser than everything except another no_value
		}
		else if (it.is_no_value())
			return false;
		else
			return _idx < it._idx; // NB: ignore _i2o part here
	}

	bool operator>(const iterator& it) const
	{
		if (is_no_value())
		{
			if (it.is_no_value())
				return false;
			else
				return false; // bottom is lesser than everything except another bottom
		}
		else if (it.is_no_value())
			return true;
		else
			return _idx > it._idx; // NB: ignore _i2o part here
	}

	bool operator>=(const iterator& it) const
	{
		return !(*this < it);
	}

	bool operator<=(const iterator& it) const
	{
		return !(*this > it);
	}

	bool is_no_value() const
	{
		return types::has_no_value(index_marker_type{_idx}) || _i2o == nullptr;
	}
};

/**
 * Maintains an indexed list of objects with ability to search a
 * object by an index and an index by a object.
 */
template<
	template<class, class> class Object2IndexT,
	template<class> class Index2ObjectT,
	class Index,
	class TwoWayObjects,
	class OneWayObjects
>
class type
{
	using all_objects_tuple = typename tuple::helper<TwoWayObjects>::template cat<OneWayObjects>;
	using tuple_helper_all = tuple::helper<all_objects_tuple>;
	using tuple_helper_1w = tuple::helper<OneWayObjects>;
	using tuple_helper_2w = tuple::helper<TwoWayObjects>;

	// 2w objects are RO, 1w - RW
	using index2objects_tuple = typename tuple::helper<typename tuple_helper_2w::template tuple_of_containers<Index2ObjectT>>::template cat<
		typename tuple_helper_1w::template tuple_of_containers<Index2ObjectT>
	>;
	using index2objects_tuple_const = typename tuple::addconst<index2objects_tuple>::type;
	
	using objects2index_tuple = typename tuple_helper_2w::template tuple_of_cref_maps<Object2IndexT, Index>;
public:
	using index_type = Index;
	using index_marker_type = marker::type<marker::index_marker, Index>;
	using iterator = multi_way_object_indexer::iterator<Index, index2objects_tuple>;
	using const_iterator = multi_way_object_indexer::iterator<Index, const index2objects_tuple_const>;
	
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;
	using value_type = typename iterator::value_type;
	using value_tuple = typename value_type::value_tuple;
	using reference = typename iterator::reference;
	using const_reference = typename iterator::const_reference;
	
	// NB no pointers declared

	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	type() {}

	type(const type&) = default;

	type(type&&) = default;
	
	template<class... Ts>
	type(std::initializer_list<std::tuple<Ts...>> objs)
	{
		for (auto it = objs.begin(); it != objs.end(); ++it)
			push_back(*it);
	}

	// allows to construct a container from different types of arguments
	template<class Arg0, class... Args>
	type(Arg0&& obj0, Args&&... objs)
	{
		push_back_seq(std::forward<Arg0>(obj0), std::forward<Args>(objs)...);
	}

	type& operator=(const type&) = default;

	type& operator=(type&&) = default;

	reference front()
	{
		return *begin();
	}
	
	const_reference front() const
	{
		return *begin();
	}
	
	reference back()
	{
		auto it = end();
		--it;
		return *it;
	}

	const_reference back() const
	{
		auto it = end();
		--it;
		return *it;
	}

	template<class... Ts>
	iterator push_back(std::tuple<Ts...>&& v)
	{
		return push_back_int(value_type::make_value_tuple(std::move(v)));
	}
	
	template<class T>
	auto push_back(T&& v)
		-> std::enable_if_t<!tuple::has_tuple_as_base<T>::value, iterator>
	{
		return push_back_int(value_type::make_value_tuple(std::forward_as_tuple(std::move(v))));
	}
	
	template<class T0, class... Ts>
	void push_back_seq(T0&& v0, Ts&&... vs)
	{
		push_back(std::forward<T0>(v0));
		push_back_seq(std::forward<Ts>(vs)...);
	}

	void push_back_seq()
	{
	}

	template<class... Ts>
	iterator push_in_hole(std::tuple<Ts...>&& v)
	{
		return push_in_hole_int(value_type::make_value_tuple(std::move(v)));
	}
	
	iterator begin() noexcept
	{
		return iterator(&_index2object_tuple, 0);
	}
	
	const_iterator begin() const noexcept
	{
		return const_iterator(reinterpret_cast<const index2objects_tuple_const*>(&_index2object_tuple), 0);
	}
	
	iterator end() noexcept
	{
		assert(_end_idx >= 0);
		return iterator(&_index2object_tuple, _end_idx);
	}
	
	const_iterator end() const noexcept
	{
		assert(_end_idx >= 0);
		return const_iterator(reinterpret_cast<const index2objects_tuple_const*>(&_index2object_tuple), _end_idx);
	}
	
	const_iterator cbegin() const noexcept
	{
		return begin();
	}
	
	const_iterator cend() const noexcept
	{
		return end();
	}

	reverse_iterator rbegin() noexcept
	{
		return std::reverse_iterator<iterator>(end());
	}
	
	reverse_iterator rend() noexcept
	{
		return std::reverse_iterator<iterator>(begin());
	}
	
	const_reverse_iterator crbegin() const noexcept
	{
		return std::reverse_iterator<const_iterator>(end());
	}
	
	const_reverse_iterator crend() const noexcept
	{
		return std::reverse_iterator<const_iterator>(begin());
	}

	bool operator==(const type& o) const noexcept
	{
		return std::equal(begin(), end(), o.begin(), o.end());
	}

	bool operator!=(const type& o) const noexcept
	{
		return !(*this == o);
	}

	void swap(type& o)
	{
		using std::swap;

		swap(_object2index_tuple, o._object2index_tuple);
		swap(_index2object_tuple, o._index2object_tuple);
		swap(_end_idx, o._end_idx);
		swap(_erased, o._erased);
	}

	size_type size() const noexcept
	{
		assert(_end_idx >= 0);
		return _end_idx;
	}

	size_type empty() const noexcept
	{
		return size() == 0;
	}

	void clear()
	{
		*this = type{};
	}
	
	iterator find(index_marker_type idx_marker)
	{
		using i2o_size_type = decltype(std::get<0>(_index2object_tuple).size());
		
		auto idx = idx_marker._value;
		
		if ((i2o_size_type) idx >= std::get<0>(_index2object_tuple).size() || idx < 0)
			return end();

		return begin() + idx;
	}

	const_iterator find(index_marker_type idx_marker) const
	{
		auto idx = idx_marker._value;
		
		if (idx >= std::get<0>(_index2object_tuple).size() || idx < 0)
			return end();

		return begin() + idx;
	}

	template<class T>
	iterator find(const T& obj)
	{
		auto& o2i = object2index<T>();
		
		const auto it = o2i.find(obj);
		if (it == o2i.end())
			return end();

		return begin() + it->second;
	}

	template<class T>
	const_iterator find(const T& obj) const
	{
		auto& o2i = object2index<T>();
		
		const auto it = o2i.find(obj);
		if (it == o2i.end())
			return end();

		return begin() + it->second;
	}

	template<class T>
	iterator strict_find(const T& obj)
	{
		return (types::has_no_value(obj)) ? iterator{} : find(obj);
	}

	template<class T>
	const_iterator strict_find(const T& obj) const
	{
		return (types::has_no_value(obj)) ? iterator{} : find(obj);
	}

	template<class T>
	auto lower_bound(const T& obj) const
	{
		return object2index<T>().lower_bound(obj);
	}

	template<class T>
	auto upper_bound(const T& obj) const
	{
		return object2index<T>().upper_bound(obj);
	}

	template<class T>
	auto equal_range(const T& obj) const
	{
		return object2index<T>().equal_range(obj);
	}

	reference operator[](index_marker_type idx)
	{
		index_type idx_int;
		if (!types::get_value(idx, idx_int))
			return reference{};
		
		auto it = begin() + idx_int;
		if (it >= end())
			return reference{};
		else
			return *it;
	}
	
	const_reference operator[](index_marker_type idx) const
	{
		index_type idx_int;
		if (!types::get_value(idx, idx_int))
			return const_reference{};
		
		auto it = begin() + idx_int;
		if (it >= end())
			return const_reference{};
		else
			return *it;
	}
	
	template<class T>
	reference operator[](const T& obj)
	{
		auto it = find(obj);
		if (it < begin() || it >= end())
			return reference{};
		else
			return *it;
	}
	
	template<
		class T,
		std::enable_if_t<!std::is_same<T, index_marker_type>::value, bool> = false
	>
	const_reference operator[](const T& obj) const
	{
		auto it = find(obj);
		if (it < begin() || it >= end())
			return const_reference{};
		else
			return *it;
	}
	
	
	template<class T>
	reference at(const T& obj)
	{
		auto it = find(obj);
		if (it < begin() || it >= end())
			throw std::out_of_range("two_way_object_indexer at()");
		else
			return *it;
	}

	template<class T>
	const_reference at(const T& obj) const
	{
		auto it = find(obj);
		if (it < begin() || it >= end())
			throw std::out_of_range("two_way_object_indexer at()");
		else
			return *it;
	}

	void erase(const iterator& it)
	{
		if (it.is_no_value())
			return; // false;

		if (it < begin() || it >= end())
			return; // false;

		_erased.push_back(it._idx);
		
		tuple_helper_2w::template for_each_no_result_forward_args<erase_functor_2w>(
			_object2index_tuple,
			_index2object_tuple,
			it._idx
		);
		tuple_helper_1w::template for_each_no_result_forward_args<erase_functor_1w>(
			_index2object_tuple,
			it._idx
		);
	}

#if 1
	template<class Key, class... Pars>
	std::pair<reference, bool> update_or_insert(const Key& key, Pars&&... pars)
	{
		bool insert = false;
		
		auto it = find(key);
		if (it == end())
		{
			push_back_int(value_type::make_value_tuple(std::forward_as_tuple(key, std::forward<Pars>(pars)...)));
			it = end() - 1;
			insert = true;
		}
		else
		{
			tuple::helper<std::tuple<Pars...>>::template for_each_no_result_forward_args<protect<TwoWayObjects>::template rewrite_functor_1w>(
				_index2object_tuple,
				std::forward_as_tuple(std::forward<Pars>(pars)...),
				(*it).template by_type<const index_marker_type>()
			);
		}
		assert((*it).template by_type<const Key>() == key);
		return std::make_pair(*it, insert);
	}

	// this one should be used to modify 2-way objects
	// it keeps the old i2o row but removes all key references to it
	// returns true if the old row was not found by `key` (so logically it is a new row)
	// NB when a map containing `key` is a multimap be ready to call Houston
	template<class Key, class... Pars>
	bool push_back_as_update(const Key& key, Pars&&... pars)
	{
		const auto old_it = find(key);
		if (old_it == end())
		{
			push_back(std::forward_as_tuple(key, std::forward<Pars>(pars)...));
			return true; 
		}

		index_type old_idx;
		if (!types::get_value((*old_it).index(), old_idx))
		{
			assert(false);
			return false;
		}
		
		// insert a new row, i2o only yet
		// NB both 1w and 2w objects
		tuple_helper_all::template for_each_no_result_forward_args<push_functor_1w>(
			_index2object_tuple,
			value_type::make_value_tuple(std::forward_as_tuple(key, std::forward<Pars>(pars)...)),
			_end_idx
		);

		tuple_helper_2w::template for_each_no_result_forward_args<update_keys_functor_2w>(
			_object2index_tuple,
			_index2object_tuple,
			old_idx,
			_end_idx
		);
		
		++_end_idx;
		
		return false;
	}
#endif
	
protected:
	iterator push_back_int(value_tuple&& tup)
	{
		// 2 way objects
		
		tuple_helper_2w::template for_each_no_result_forward_args<push_functor_2w>(
			_object2index_tuple,
			_index2object_tuple,
			std::move(tup),
			_end_idx
		);

		// 1 way objects

		tuple_helper_1w::template for_each_no_result_forward_args<push_functor_1w>(
			_index2object_tuple,
			std::move(tup),
			_end_idx
		);

		return iterator(&_index2object_tuple, _end_idx++);
	}
	
	iterator push_in_hole_int(value_tuple&& tup)
	{
		if (_erased.empty()) { // there is no hole, push back
			return push_back_int(std::move(tup));
		}
		
		const index_type idx = _erased.back();
		_erased.pop_back();

		tuple_helper_2w::template for_each_no_result_forward_args<rewrite_functor_2w>(
			_object2index_tuple,
			_index2object_tuple,
			std::move(tup),
			idx
		);

		// 1 way objects

		tuple_helper_1w::template for_each_no_result_forward_args<protect<TwoWayObjects>::template rewrite_functor_1w>(
			_index2object_tuple,
			std::move(tup),
			index_marker_type{idx}
		);

		return iterator(&_index2object_tuple, idx);
	}
	
	template<class T>
	Object2IndexT<std::reference_wrapper<const T>, Index>& object2index()
	{
		static_assert(
			tuple::among_tuple_types<T, TwoWayObjects>::value,
			"Unable to index the collection by the type not specified in TwoWayObjects"
		);
		return std::get<Object2IndexT<std::reference_wrapper<const T>, Index>>(_object2index_tuple);
	}

	template<class T>
	const Object2IndexT<std::reference_wrapper<const T>, Index>& object2index() const
	{
		static_assert(
			tuple::among_tuple_types<T, TwoWayObjects>::value,
			"Unable to index the collection by the type not specified in TwoWayObjects"
		);
		return std::get<Object2IndexT<std::reference_wrapper<const T>, Index>>(_object2index_tuple);
	}

	template<class T>
	Index2ObjectT<T>& index2object()
	{
		return std::get<Index2ObjectT<T>>(_index2object_tuple);
	}

	template<class T>
	const Index2ObjectT<std::reference_wrapper<const T>>& index2object() const
	{
		return std::get<Index2ObjectT<T>>(_index2object_tuple);
	}

private:
	objects2index_tuple _object2index_tuple;
	index2objects_tuple _index2object_tuple;
	index_type _end_idx = 0;
	std::list<index_type> _erased;
};


} // namespace multi_way_object_indexer

template<class Key>
struct ref_hash : marker::hash<Key> {};

template<class K>
struct ref_hash<std::reference_wrapper<K>>
{
	using hash = marker::hash<K>;

	std::size_t operator()(const K& k) const { return hash()(k); }
};

template<class K>
struct ref_hash<std::reference_wrapper<const K>>
{
	using hash = marker::hash<K>;

	std::size_t operator()(const K& k) const { return hash()(k); }
};

template<class T>
struct ref_less : std::less<T> {};

template<class T>
struct ref_less<T&>
{
	using less = std::less<T>;

	bool operator()(const T& a, const T& b) const { return less()(a, b); }
};

template<class T>
struct ref_less<std::reference_wrapper<T>>
{
	using less = std::less<T>;

	bool operator()(const T& a, const T& b) const { return less()(a, b); }
};

template<class T>
struct ref_equal_to : std::equal_to<T> {};

template<class T>
struct ref_equal_to<std::reference_wrapper<T>>
{
	using equal_to = std::equal_to<T>;
	
	bool operator()(const T& a, const T& b) const { return equal_to()(a, b); }
};


// all default parameters except Key and Value
template<class Key, class T>
using map = std::map<Key, T, ref_less<Key>>;

template<class Key, class T>
using multimap = std::multimap<Key, T, ref_less<Key>>;

template<class Key, class T>
using unordered_map = std::unordered_map<Key, T, ref_hash<Key>, ref_equal_to<Key>>;

// Don't use vector! (we should guarantee constant memory position with elements)
template<class T>
using deque = std::deque<T>;

/* std::back_insert_iterator extension */

template<class Container>
class back_insert_iterator :
		public std::iterator<std::output_iterator_tag, void, void, void, void>
{
public:
	constexpr back_insert_iterator() noexcept : _c(nullptr) {}

	explicit back_insert_iterator(Container& c) : _c(&c) {}

	back_insert_iterator& operator*() { return *this; }

	back_insert_iterator& operator++() { return *this; }

	back_insert_iterator& operator++(int) { return *this; }

	template<class T>
	back_insert_iterator& operator=(T&& v)
	{
		assert(_c);
		_c->push_back(std::forward<T>(v));
		return *this;
	}
	
protected:
	Container* _c;
};

template<class Container>
back_insert_iterator<Container> back_inserter(Container& c)
{
	return back_insert_iterator<Container>(c);
}

} // namespace map
