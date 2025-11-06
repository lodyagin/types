/**
 * @file
 * This is a helper for mapping a memory as a container.
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

// This represents irregular cells sequence

#ifndef TYPES_SEQUENCE_H
#define TYPES_SEQUENCE_H

#include <iterator>
#include <utility>

namespace types
{

namespace forward_sequence
{

#ifndef TYPES_SEQUENCE_NO_PRELOADER
struct no_preloader
{
	constexpr const void* operator()(const void*) const { return nullptr; }
};
#endif

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = no_preloader
#endif
>
class const_iterator
{
public:
	using navigator_type = Navigator;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	using preloader_type = Preloader;
#endif
	using size_type = typename navigator_type::size_type;
	using difference_type = typename navigator_type::difference_type;
	using value_type = typename navigator_type::value_type;
	using pointer = const value_type*;
	using reference = const value_type&;
	
	bool is_valid() const noexcept
	{
		return _address != no_address()
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			&& _stop_address != no_address()
#endif
			&& navigator_type::is_valid_cell(
				_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
				, _stop_address
#endif
			);
	}
	
	reference operator*() noexcept
	{
		return *_address;
	}

	reference operator*() const noexcept
	{
		return *_address;
	}

	const_iterator& operator++()
	{
		const pointer new_address = navigator_type::forward(_address);
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		if (new_address > _address && new_address <= _stop_address)
			_address = new_address;
		else
			_address = _stop_address; // to prevent an infinite loop
#else
		_address = new_address;
#endif

#ifndef TYPES_SEQUENCE_NO_PRELOADER
		if (_preloader != nullptr && _address > _preload_stop)
			_preload_stop = (*_preloader)(_address);
#endif
		
		return *this;
	}

	const_iterator operator++(int)
	{
		const_iterator copy = *this;
		++(*this);
		return copy;
	}

	constexpr const_iterator() noexcept {}

#ifndef TYPES_SEQUENCE_NO_PRELOADER
	const void* dummy_preloader(const void*) { return nullptr; }
#endif
	
	constexpr bool operator==(const const_iterator& o) const noexcept
	{
		return _address == o._address;
	}

	constexpr bool operator!=(const const_iterator& o) const noexcept
	{
		return !operator==(o);
	}

	pointer operator->() const noexcept
	{
		return _address;
	}

	static constexpr pointer no_address()
	{
		return (pointer) navigator_type::no_address();
	}

	// extended definitions
	
	const_iterator(
		pointer addr
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		, pointer stop_addr
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, const preloader_type* preloader
#endif
	) noexcept
		: _address(addr)
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		, _stop_address(stop_addr)
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, _preloader(preloader)
#endif
	{
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		if (_preloader != nullptr && _address != no_address())
			_preload_stop = (*_preloader)(_address);
#endif
	}

	const_iterator shifted_iterator(std::ptrdiff_t shift) const
	{
		return const_iterator(
			(pointer)((const char*) _address + shift)
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, _stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, _preloader
#endif
		);
	}

protected:
	pointer _address = no_address();
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
	pointer _stop_address = no_address();
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	const preloader_type* _preloader = nullptr;
	const void* _preload_stop = nullptr;
#endif
};

// Is reqiured for the following concept test
struct char_navigator
{
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using value_type = char;
	//using iterator_category = std::forward_iterator_tag;

	static const char* forward(const char* cur) noexcept
	{
		return cur + 1;
	}

	static constexpr const void* no_address() { return nullptr; }
};

static_assert(
	std::forward_iterator<
    const_iterator<
	    char_navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	    , no_preloader
#endif
	  >
	>
);

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = no_preloader
#endif
>
class type
{
public:
	using navigator_type = Navigator;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	using preloader_type = Preloader;
#endif
	
	// Container part
	
	using const_iterator = forward_sequence::const_iterator<
		navigator_type
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, preloader_type
#endif
	>;
	using iterator = const_iterator;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;
	using pointer = typename iterator::pointer;
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;

	constexpr type(
		pointer start,
		pointer stop
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, preloader_type preloader = no_preloader{}
#endif
	) noexcept
		: _start_address(start), _stop_address(stop)
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, _preloader(preloader)
#endif
	{}
	
	iterator begin() const noexcept
	{
		return iterator(
			_start_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, _stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, &_preloader
#endif
		);
	}
	
	iterator end() const noexcept
	{
		return iterator(
			_stop_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, _stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, nullptr
#endif
		);
	}

	const_iterator cbegin() const noexcept { return begin(); }

	const_iterator cend() const noexcept { return end(); }

	// Attention: on the first invocation causes reading the whole file !
	size_type size() const
	{
		static const size_calculator sc{*this};
		return sc._size;
	}
	
	bool empty() const noexcept { return cbegin() == cend(); }

	std::pair<pointer, pointer> context(const_iterator it)
	{
		//		return navigator_type::range(&*it);
		return std::make_pair(&*it, _start_address);
	}
	
protected:
	pointer _start_address = nullptr;
	pointer _stop_address = nullptr;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	preloader_type _preloader;
#endif
	
	struct size_calculator
	{
		size_type _size = 0;

		size_calculator(const type& cont)
		{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
			for (const auto& b : cont)
				++_size;
#pragma GCC diagnostic pop
		}
	};
};

} // namespace forward_sequence
 
namespace bidirectional_sequence
{

//typedef const void* (*preload_callback)(const void* start);

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = forward_sequence::no_preloader
#endif
>
class const_iterator : public forward_sequence::const_iterator<
	Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, Preloader
#endif
>
{
	using base = forward_sequence::const_iterator<
		Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, Preloader
#endif
	>;
	
public:
	using navigator_type = Navigator;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	using preloader_type = Preloader;
#endif
	using size_type = typename base::size_type;
	using difference_type = typename base::difference_type;
	using value_type = typename base::value_type;
	using pointer = typename base::pointer;
	using reference = typename base::reference;

	using base::base;
	
	using base::no_address;

	const_iterator& operator++()
	{
		return static_cast<const_iterator&>(base::operator++());
	}

	const_iterator operator++(int)
	{
		return static_cast<const_iterator>(base::operator++(0));
	}
	
	const_iterator& operator--()
	{
		const pointer new_address = navigator_type::backward(this->_address);
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		if (new_address < this->_address && new_address >= this->_start_address)
			this->_address = new_address;
		else
			this->_address = this->_start_address; // to prevent an infinite loop
#else
		this->_address = new_address;
#endif
		
		return *this;
	}

	const_iterator operator--(int)
	{
		const auto copy = *this;
		--(*this);
		return copy;
	}

	const_iterator(
		pointer addr
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		, pointer start_addr
		, pointer stop_addr
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, const preloader_type* preloader
#endif
	) noexcept
		: base::const_iterator(
			  addr
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
				, stop_addr
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
				, preloader
#endif
		  )
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		  , _start_address(start_addr)
#endif
	{
	}
	
protected:
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
	pointer _start_address = no_address();
#endif
};

static_assert(
	std::bidirectional_iterator<
	   const_iterator<
	      forward_sequence::char_navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	      , forward_sequence::no_preloader
#endif
	   >
	>
);

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = forward_sequence::no_preloader
#endif
>
class type : public forward_sequence::type<
	Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, Preloader
#endif
>
{
	using base = forward_sequence::type<
		Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, Preloader
#endif
	>;
public:
	using navigator_type = Navigator;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	using preloader_type = Preloader;
#endif

	using const_iterator = bidirectional_sequence::const_iterator<
		navigator_type
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, preloader_type
#endif
	>;
	using iterator = const_iterator;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;
	using pointer = typename iterator::pointer;
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;

	using base::base;

	iterator begin() const noexcept
	{
		return iterator(
			this->_start_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, this->_start_address
			, this->_stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, &this->_preloader
#endif
		);
	}
	
	iterator end() const noexcept
	{
		return iterator(
			this->_stop_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, this->_start_address
			, this->_stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, nullptr
#endif
		);
	}

	const_iterator cbegin() const noexcept { return begin(); }

	const_iterator cend() const noexcept { return end(); }

};

} // namespace bidirectional_sequence
 
namespace random_access_sequence
{

//typedef const void* (*preload_callback)(const void* start);

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = forward_sequence::no_preloader
#endif
>
class const_iterator : public bidirectional_sequence::const_iterator<
	Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, Preloader
#endif
>
{
	using base = bidirectional_sequence::const_iterator<
		Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, Preloader
#endif
	>;
	
public:
	using navigator_type = Navigator;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	using preloader_type = Preloader;
#endif
	using size_type = typename base::size_type;
	using difference_type = typename base::difference_type;
	using value_type = typename base::value_type;
	using pointer = typename base::pointer;
	using reference = typename base::reference;

	using base::base;
	using base::no_address;
	
	const_iterator& operator++()
	{
		return static_cast<const_iterator&>(base::operator++());
	}

	const_iterator operator++(int)
	{
		return static_cast<const_iterator>(base::operator++(0));
	}
	
	const_iterator& operator--()
	{
		return static_cast<const_iterator&>(base::operator--());
	}

	const_iterator operator--(int)
	{
		return static_cast<const_iterator>(base::operator--(0));
	}
	
	const_iterator& operator+=(difference_type n)
	{
		const pointer new_address = navigator_type::forward(this->_address, n);
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
		if (new_address > this->_address) {
			if (new_address <= this->_stop_address) {
				this->_address = new_address;

#ifndef TYPES_SEQUENCE_NO_PRELOADER
				if (this->_preloader != nullptr && this->_address > this->_preload_stop)
					this->_preload_stop = (*this->_preloader)(this->_address);
#endif
			}
			else
				this->_address = this->_stop_address;
		}
		else if (new_address <= this->_address) {
			if (new_address >= this->_start_address)
				this->_address = new_address;
			else
				this->_address = this->_start_address;
		}
		else
			this->_address = this->_stop_address; // to prevent an infinite loop
#else
		this->_address = new_address;
#endif
		
		return *this;
	}

	const_iterator operator+(difference_type n) const
	{
		auto copy = *this;
		return copy += n;
	}
	
	const_iterator& operator-=(difference_type n)
	{
		return operator+=(-n);
	}

	const_iterator operator-(difference_type n) const
	{
		auto copy = *this;
		return copy -= n;
	}

	difference_type operator-(const const_iterator& b) const
	{
		return this->_address - b._address;
	}

	reference operator[](difference_type n)
	{
		return *(*this + n);
	}

	reference operator[](difference_type n) const
	{
		return *(*this + n);
	}

	constexpr bool operator<(const const_iterator& b) const noexcept
	{
		if (b._address == no_address()) {
			return false;
		}
		else {
			if (this->_address == no_address())
				return true;
			else
				return this->_address < b._address;
		}
	}

	constexpr bool operator>(const const_iterator& b) const noexcept
	{
		if (this->_address == no_address()) {
			return false;
		}
		else {
			if (b._address == no_address())
				return true;
			else
				return this->_address < b._address;
		}
	}

	constexpr bool operator<=(const const_iterator& b) const noexcept
	{
		return !operator>(b);
	}
	
	constexpr bool operator>=(const const_iterator& b) const noexcept
	{
		return !operator<(b);
	}
};

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = forward_sequence::no_preloader
#endif
>
const_iterator<
	Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, Preloader
#endif
> operator+(
	typename const_iterator<
	   Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	   , Preloader
#endif
	>::difference_type n,
	const const_iterator<
	   Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	   , Preloader
#endif
	>&a
)
{
	return a + n;
}

static_assert(
	std::random_access_iterator<
	   const_iterator<
	      forward_sequence::char_navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	      , forward_sequence::no_preloader
#endif
	   >
	>
);

template<
	class Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, class Preloader = forward_sequence::no_preloader
#endif
>
class type : public bidirectional_sequence::type<
	Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	, Preloader
#endif
>
{
	using base = bidirectional_sequence::type<
		Navigator
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, Preloader
#endif
	>;
public:
	using navigator_type = Navigator;
#ifndef TYPES_SEQUENCE_NO_PRELOADER
	using preloader_type = Preloader;
#endif

	using const_iterator = random_access_sequence::const_iterator<
		navigator_type
#ifndef TYPES_SEQUENCE_NO_PRELOADER
		, preloader_type
#endif
	>;
	using iterator = const_iterator;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;
	using pointer = typename iterator::pointer;
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;

	using base::base;
	
	iterator begin() const noexcept
	{
		return iterator(
			this->_start_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, this->_start_address
			, this->_stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, &this->_preloader
#endif
		);
	}
	
	iterator end() const noexcept
	{
		return iterator(
			this->_stop_address
#ifndef TYPES_SEQUENCE_NO_RANGE_CHECK
			, this->_start_address
			, this->_stop_address
#endif
#ifndef TYPES_SEQUENCE_NO_PRELOADER
			, nullptr
#endif
		);
	}

	const_iterator cbegin() const noexcept { return begin(); }

	const_iterator cend() const noexcept { return end(); }

	size_type size() const
	{
		return cend() - cbegin();
	}
};

} // namespace random_access_sequence

namespace congiguous_sequence
{



} // namespace contiguous_sequence

} // namespace types

#endif // TYPES_SEQUENCE_H
