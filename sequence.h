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

//typedef const void* (*preload_callback)(const void* start);

template<class Navigator, class Preloader>
class const_iterator
{
public:
	// LeagacyIterator part
	
	using navigator_type = Navigator;
	using preloader_type = Preloader;
	using size_type = typename navigator_type::size_type;
	using difference_type = typename navigator_type::difference_type;
	using value_type = typename navigator_type::value_type;
	using pointer = const value_type*;
	using reference = const value_type&;
	//using iterator_category = typename navigator_type::iterator_category;
	
	bool is_valid() const noexcept
	{
		return _address != no_address() && _stop_address != no_address()
			&& navigator_type::is_valid_cell(_address, _stop_address);
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
		if (new_address > _address && new_address <= _stop_address)
			_address = new_address;
		else
			_address = _stop_address; // to prevent an infinite loop

		if (_preloader != nullptr && _address > _preload_stop)
			_preload_stop = (*_preloader)(_address);
		
		return *this;
	}

	const_iterator operator++(int)
	{
		const_iterator copy = *this;
		++(*this);
		return copy;
	}

	// LeagacyForwardIterator part
	
	constexpr const_iterator() noexcept {}

	const void* dummy_preloader(const void*) { return nullptr; }
	
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
	
	const_iterator(pointer addr, pointer stop_addr, const preloader_type* preloader) noexcept
		: _address(addr), _stop_address(stop_addr), _preloader(preloader)
	{
		if (_preloader != nullptr && _address != no_address())
			_preload_stop = (*_preloader)(_address);
	}

	const_iterator shifted_iterator(std::ptrdiff_t shift) const
	{
		return const_iterator(
			(pointer)((const char*) _address + shift),
			_stop_address,
			_preloader
		);
	}

protected:
	pointer _address = no_address();
	pointer _stop_address = no_address();
	const preloader_type* _preloader = nullptr;
	const void* _preload_stop = nullptr;
};

struct no_preloader
{
	constexpr const void* operator()(const void*) const { return nullptr; }
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

static_assert(std::forward_iterator<const_iterator<char_navigator, no_preloader>>);

template<class Navigator, class Preloader = no_preloader>
class type
{
public:
	using navigator_type = Navigator;
	using preloader_type = Preloader;
	
	// Container part
	
	using const_iterator = forward_sequence::const_iterator<navigator_type, preloader_type>;
	using iterator = const_iterator;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;
	using pointer = typename iterator::pointer;
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;

	constexpr type(pointer start, pointer stop, preloader_type preloader = no_preloader{}) noexcept
		: _start_address(start), _stop_address(stop), _preloader(preloader)
	{}
	
	iterator begin() const noexcept
	{
		return iterator(_start_address, _stop_address, &_preloader);
	}
	
	iterator end() const noexcept
	{
		return iterator(_stop_address, _stop_address, nullptr);
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
	preloader_type _preloader;
	
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

template<class Navigator, class Preloader>
class const_iterator : public forward_sequence::const_iterator<Navigator, Preloader>
{
	using base = forward_sequence::const_iterator<Navigator, Preloader>;
public:
	using navigator_type = Navigator;
	using preloader_type = Preloader;
	using size_type = typename base::size_type;
	using difference_type = typename base::difference_type;
	using value_type = typename base::value_type;
	using pointer = typename base::pointer;
	using reference = typename base::reference;
	//using iterator_category = typename base::iterator_category;

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
		if (new_address < this->_address && new_address >= this->_start_address)
			this->_address = new_address;
		else
			this->_address = this->_start_address; // to prevent an infinite loop
		
		return *this;
	}

	const_iterator operator--(int)
	{
		const auto copy = *this;
		--(*this);
		return copy;
	}

	const_iterator(
		pointer addr,
		pointer start_addr,
		pointer stop_addr,
		const preloader_type* preloader
	) noexcept
		: base::const_iterator(addr, stop_addr, preloader),
			_start_address(start_addr)
	{
	}
	
protected:
	pointer _start_address = no_address();
};

static_assert(std::bidirectional_iterator<const_iterator<forward_sequence::char_navigator, forward_sequence::no_preloader>>);

template<class Navigator, class Preloader = forward_sequence::no_preloader>
class type : public forward_sequence::type<Navigator, Preloader>
{
	using base = forward_sequence::type<Navigator, Preloader>;
public:
	using navigator_type = Navigator;
	using preloader_type = Preloader;

	using const_iterator = bidirectional_sequence::const_iterator<navigator_type, preloader_type>;
	using iterator = const_iterator;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;
	using pointer = typename iterator::pointer;
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;

	using base::base;

	iterator begin() const noexcept
	{
		return iterator(this->_start_address, this->_start_address, this->_stop_address, &this->_preloader);
	}
	
	iterator end() const noexcept
	{
		return iterator(this->_stop_address, this->_start_address, this->_stop_address, nullptr);
	}

	const_iterator cbegin() const noexcept { return begin(); }

	const_iterator cend() const noexcept { return end(); }

};

} // namespace bidirectional_sequence
 
namespace random_access_sequence
{

//typedef const void* (*preload_callback)(const void* start);

template<class Navigator, class Preloader>
class const_iterator : public bidirectional_sequence::const_iterator<Navigator, Preloader>
{
	using base = bidirectional_sequence::const_iterator<Navigator, Preloader>;
public:
	using navigator_type = Navigator;
	using preloader_type = Preloader;
	using size_type = typename base::size_type;
	using difference_type = typename base::difference_type;
	using value_type = typename base::value_type;
	using pointer = typename base::pointer;
	using reference = typename base::reference;
	//using iterator_category = typename base::iterator_category;

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
		if (new_address > this->_address) {
			if (new_address <= this->_stop_address) {
				this->_address = new_address;

				if (this->_preloader != nullptr && this->_address > this->_preload_stop)
					this->_preload_stop = (*this->_preloader)(this->_address);
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

template<class Navigator, class Preloader>
const_iterator<Navigator, Preloader> operator+(typename const_iterator<Navigator, Preloader>::difference_type n, const const_iterator<Navigator, Preloader>&a)
{
	return a + n;
}

static_assert(std::random_access_iterator<const_iterator<forward_sequence::char_navigator, forward_sequence::no_preloader>>);

template<class Navigator, class Preloader = forward_sequence::no_preloader>
class type : public bidirectional_sequence::type<Navigator, Preloader>
{
	using base = bidirectional_sequence::type<Navigator, Preloader>;
public:
	using navigator_type = Navigator;
	using preloader_type = Preloader;

	using const_iterator = random_access_sequence::const_iterator<navigator_type, preloader_type>;
	using iterator = const_iterator;
	using reference = typename iterator::reference;
	using const_reference = typename const_iterator::reference;
	using pointer = typename iterator::pointer;
	using difference_type = typename iterator::difference_type;
	using size_type = typename iterator::size_type;

	using base::base;
	
	iterator begin() const noexcept
	{
		return iterator(this->_start_address, this->_start_address, this->_stop_address, &this->_preloader);
	}
	
	iterator end() const noexcept
	{
		return iterator(this->_stop_address, this->_start_address, this->_stop_address, nullptr);
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
