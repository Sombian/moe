#pragma once

#include <bit>
#include <string>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <ostream>
#include <utility>
#include <type_traits>

#include "traits/printable.hpp"
#include "traits/rule_of_5.hpp"

#include "utils/ordering.hpp"

//|----------------------------------------------------------------------------------------|
//| special thanks to facebook's folly::FBString.                                          |
//|                                                                                        |
//| SSO mode uses every bytes of heap string struct using union.                           |
//| this was achievable, thanks to the very clever memory layout.                          |
//|                                                                                        |
//| c-style string has null-terminator at the end to denote the end of the string.         |
//| here, spare capacity is written to the RMB, and once fully occupied, RMB becomes zero. |
//|----------------------------------------------------------------------------------------|

auto operator<<(std::ostream& os, const char32_t code) -> std::ostream&
{
	char data[5]
	{
		0, // <- null terminator
		0, // <- null terminator
		0, // <- null terminator
		0, // <- null terminator
		0, // <- null terminator
	};

	char* ptr {data};

	if (code < 0x7F)
	{
		*(ptr++) = code;
	}
	else if (code < 0x7FF)
	{
		*(ptr++) = 0xC0 | ((code >> 06) & 0x1F);
		*(ptr++) = 0x80 | ((code >> 00) & 0x3F);
	}
	else if (code < 0xFFFF)
	{
		*(ptr++) = 0xE0 | ((code >> 12) & 0x0F);
		*(ptr++) = 0x80 | ((code >> 06) & 0x3F);
		*(ptr++) = 0x80 | ((code >> 00) & 0x3F);
	}
	else if (code < 0x10FFFF)
	{
		*(ptr++) = 0xF0 | ((code >> 18) & 0x07);
		*(ptr++) = 0x80 | ((code >> 12) & 0x3F);
		*(ptr++) = 0x80 | ((code >> 06) & 0x3F);
		*(ptr++) = 0x80 | ((code >> 00) & 0x3F);
	}
	*ptr = '\0';
	os << data;
	return os;
}

template
<
	typename T
>
requires
(
	std::is_same_v<T, char8_t>
	||
	std::is_same_v<T, char16_t>
	||
	std::is_same_v<T, char32_t>
)
class text
{
	#define IS_BIG (std::endian::native == std::endian::big)

	enum tag : uint8_t
	{
		SMALL = IS_BIG ? 0b0000000'0 : 0b0'0000000, // always 0
		LARGE = IS_BIG ? 0b0000000'1 : 0b1'0000000, // always 1
	};

	class buffer
	{
		static constexpr const uint8_t TOTAL_BITS {sizeof(size_t) * 8};
		static constexpr const uint8_t METADATA_BITS {sizeof(uint8_t) * 8};
		static constexpr const uint8_t CAPACITY_BITS {TOTAL_BITS - METADATA_BITS};

	public:

		T* data;
		size_t size;
		size_t capacity : CAPACITY_BITS;
		size_t metadata : METADATA_BITS;

		[[nodiscard]]
		auto begin() -> T*
		{
			return this->data; // from start
		}

		[[nodiscard]]
		auto end() -> T*
		{
			return this->data + this->size;
		}

		[[nodiscard]]
		auto begin() const -> const T*
		{
			return this->data; // from start
		}

		[[nodiscard]]
		auto end() const -> const T*
		{
			return this->data + this->size;
		}

		// syntax sugar
		auto operator[](const size_t nth) -> T&
		{
			return this->data[nth];
		}

		// syntax sugar
		auto operator[](const size_t nth) const -> const T&
		{
			return this->data[nth];
		}
	};

	static_assert(std::is_standard_layout_v<buffer>, "use other compiler");
	static_assert(std::is_trivially_copyable_v<buffer>, "use other compiler");
	static_assert(sizeof(buffer) == sizeof(size_t) * 3, "use other compiler");
	static_assert(alignof(buffer) == alignof(size_t) * 1, "use other compiler");
	static_assert(offsetof(buffer, data) == sizeof(size_t) * 0, "use other compiler");
	static_assert(offsetof(buffer, size) == sizeof(size_t) * 1, "use other compiler");

	static constexpr const uint8_t MAX {(sizeof(buffer) - 1) / (sizeof(T))};
	static constexpr const uint8_t RMB {(sizeof(buffer) - 1) * (    1    )};

	static constexpr const uint8_t SFT {IS_BIG ? (    1    ) : (    0    )};
	static constexpr const uint8_t MSK {IS_BIG ? 0b0000000'1 : 0b1'0000000};

	#undef IS_BIG

	//|---------------------------------|
	//|              bytes              |
	//|---------------------------------|
	//|              small              |
	//|----------|----------|-----------|
	//|   data   |   size   |   extra   |
	//|----------|----------|-----------|

	#define unit T
	union
	{
		//|-----------------------------------|
		unit small                          //|
		[sizeof(buffer) / sizeof(unit)];    //|
		//|-----------------------------------|
		buffer large;                       //|
		//|-----------------------------------|
		uint8_t bytes                       //|
		[sizeof(buffer) / sizeof(uint8_t)]; //|
		//|-----------------------------------|
	};
	#undef unit

	//|-------------------------------------------------------|-------|
	//|                                                       |   ↓   |
	//|---------------|-------|-------|-------|-------|-------|-------|
	//| 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit |
	//|-------|-------|-------|-------|-------|-------|-------|-------|

	[[nodiscard]]
	// getter
	auto mode() const -> tag
	{
		return static_cast<tag>(this->bytes[RMB] & MSK);
	}

public:

	//|-------------------------------------------------------|-------|
	//|   ↓       ↓       ↓       ↓       ↓       ↓       ↓   |       |
	//|---------------|-------|-------|-------|-------|-------|-------|
	//| 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit |
	//|-------|-------|-------|-------|-------|-------|-------|-------|

	[[nodiscard]]
	// getter
	auto size() const -> size_t
	{
		switch (this->mode())
		{
			case tag::SMALL:
			{
				//|------------|    |---[big endian]---|
				//| 0bXXXXXXX0 | -> | skip right 1 bit |
				//|------------|    |------------------|

				//|------------|    |-[little endian]-|
				//| 0b0XXXXXXX | -> | no need to skip |
				//|------------|    |-----------------|

				return MAX - (this->bytes[RMB] >> SFT);
			}
			case tag::LARGE:
			{
				// "it just works" - its in the middle

				return this->large.size; // ¯\_(ツ)_/¯
			}
		}
		assert(false && "-Wreturn-type");
	}

	[[nodiscard]]
	// setter
	auto size(const size_t value)
	{
		if (this->capacity() < value)
		{
			allocate:
			auto data {new T[value]};

			switch (this->mode())
			{
				// S -> L
				case tag::SMALL:
				{
					std::copy(std::begin(this->small), std::end(this->small), data);
					// delete[] this->small;
					break;
				}
				// L -> L
				case tag::LARGE:
				{
					std::copy(std::begin(this->large), std::end(this->large), data);
					delete[] this->large.data;
					break;
				}
			}
			this->large.data = data;
			this->large.size = value;
			this->large.capacity = value;
			this->large.metadata = tag::LARGE;
			//|---------------------------------------------------------------------|
			assert(this->mode() == tag::LARGE); // <- make sure we're on large mode |
			//|---------------------------------------------------------------------|
		}
		else // no need for extra capacity
		{
			switch (this->mode())
			{
				// L -> L
				case tag::LARGE:
				{
					// terminate
					this->large[value] = '\0';
					// write size
					this->large.size = value;
					break;
				}
				// S -> S
				case tag::SMALL:
				{
					auto slot {MAX - value};
					// terminate
					this->small[value] = '\0';

					//|------------|    |---[big endian]---|
					//| 0bXXXXXXX0 | -> | skip right 1 bit |
					//|------------|    |------------------|

					//|------------|    |-[little endian]-|
					//| 0b0XXXXXXX | -> | no need to skip |
					//|------------|    |-----------------|

					this->bytes[RMB] = slot << SFT;
					//|---------------------------------------------------------------------|
					assert(this->mode() == tag::SMALL); // <- make sure we're on small mode |
					//|---------------------------------------------------------------------|
					break;
				}
			}
		}
		assert(this->mode() == tag::SMALL ? this->size() == value : true);
		assert(this->mode() == tag::LARGE ? this->size() == value : true);
	}

	[[nodiscard]]
	// getter
	auto capacity() const -> size_t
	{
		switch (this->mode())
		{
			case tag::SMALL:
			{
				return MAX; // always MAX
			}
			case tag::LARGE:
			{
				return this->large.capacity;
			}
		}
		assert(false && "-Wreturn-type");
	}

	[[nodiscard]]
	// setter
	auto capacity(const size_t value)
	{
		if (this->capacity() < value)
		{
			allocate:
			auto data {new T[value]};
			auto size {this->size()};

			switch (this->mode())
			{
				// S -> L
				case tag::SMALL:
				{
					std::copy(std::begin(this->small), std::end(this->small), data);
					// delete[] this->small;
					break;
				}
				// L -> L
				case tag::LARGE:
				{
					std::copy(std::begin(this->large), std::end(this->large), data);
					delete[] this->large.data;
					break;
				}
			}
			this->large.data = data;
			this->large.size = size;
			this->large.capacity = value;
			this->large.metadata = tag::LARGE;
			//|---------------------------------------------------------------------|
			assert(this->mode() == tag::LARGE); // <- make sure we're on large mode |
			//|---------------------------------------------------------------------|
		}
		else // no need for extra capacity
		{
			switch (this->mode())
			{
				// L -> L
				case tag::LARGE:
				{
					goto allocate;
				}
				// S -> S
				case tag::SMALL:
				{
					// TODO: none
					break;
				}
			}
		}
		assert(this->mode() == tag::SMALL ? this->capacity() == MAX : true);
		assert(this->mode() == tag::LARGE ? this->capacity() == value : true);
	}

private:

	template
	<
		typename P
	>
	requires
	(
		std::is_same_v<P, text<T>&>
		||
		std::is_same_v<P, text<T> const&>
	)
	class proxy
	{
		P src;
		size_t nth;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		proxy
		(
			decltype(src) src,
			decltype(nth) nth
		)
		: src(src), nth(nth) {}

		//|-----------------|
		//| member function |
		//|-----------------|

		operator char32_t() const&&
		{
			auto ptr {this->src.c_str()};

			size_t i {0};
			size_t j {0};

			if constexpr (std::is_same_v<T, char32_t>)
			{
				i = this->nth;
				j = this->nth;

				if (this->nth < this->src.size())
				{
					goto for_utf32; // <- O(1)
				}
				// no exception
				return U'\0';
			}

			for (; ptr[i]; ++j)
			{
				if (j == this->nth)
				{
					for_utf32:
					char32_t out;
					// write to ref
					codec::decode(ptr + i, out, codec::next(ptr + i));
					// ¯\_(ツ)_/¯
					return out;
				}
				i += codec::next(ptr + i);
			}
			// no exception
			return U'\0';
		}

		auto operator=(const char32_t code)&& -> proxy& requires
		(
			!std::is_const_v<std::remove_reference_t<P>>
		)
		{
			auto ptr {this->src.c_str()};

			size_t i {0};
			size_t j {0};

			if constexpr (std::is_same_v<T, char32_t>)
			{
				i = this->nth;
				j = this->nth;

				if (this->nth < this->src.size())
				{
					goto for_utf32; // <- O(1)
				}
				// no exception
				return *this;
			}

			for (; ptr[i]; ++j)
			{
				if (j == this->nth)
				{
					for_utf32:
					const auto a {codec::next(ptr + i)};
					const auto b {codec::width(code)};

					switch (utils::cmp(a, b))
					{
						//|---|--------------|
						//| a | source range |
						//|---|---|----------|---|
						//|   b   | source range |
						//|-------|--------------|
						case utils::ordering::LESS:
						{
							const size_t old_l {this->src.size()};
							const size_t new_l {old_l + (b - a)};

							if (this->src.capacity() < new_l)
							{
								this->src.capacity(new_l * 2);
							}
							std::copy_backward
							(
								ptr + i + b,
								ptr + old_l,
								ptr + i + a
							);
							this->src.size(new_l);
							break;
						}
						//|---|--------------|
						//| a | source range |
						//|---|--------------|
						//| b | source range |
						//|---|--------------|
						case utils::ordering::EQUAL:
						{
							break;
						}
						//|-------|--------------|
						//|   a   | source range |
						//|---|---|----------|---|
						//| b | source range |
						//|---|--------------|
						case utils::ordering::GREATER:
						{
							const size_t old_l {this->src.size()};
							const size_t new_l {old_l - (a - b)};

							// if (this->src.capacity() < new_l)
							// {
							// 	this->src.capacity(new_l * 2);
							// }
							std::copy
							(
								ptr + i + b,
								ptr + old_l,
								ptr + i + a
							);
							this->src.size(new_l);
							break;
						}
					}
					codec::encode(code, ptr + i, a);
				}
				i += this->src.width(ptr[i]);
			}
			return *this;
		}
	};

	class iterator
	{
		const T* ptr;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		iterator(decltype(ptr) ptr) : ptr(ptr) {}

		//|-----------------|
		//| member function |
		//|-----------------|

		auto operator&() const -> const T*
		{
			return this->ptr;
		}

		auto operator*() const -> char32_t
		{
			char32_t out;
			// write to ref
			codec::decode(ptr, out, codec::next(this->ptr));
			// ¯\_(ツ)_/¯
			return out;
		}

		// prefix (++it)
		auto operator++() -> iterator&
		{
			// += is fine because [0 < codex::next]
			this->ptr += codec::next(this->ptr);

			return *this;
		}

		// postfix (it++)
		auto operator++(int) -> iterator
		{
			auto temp {*this};
			operator++();
			return temp;
		}

		// prefix (--it)
		auto operator--() -> iterator&
		{
			// += is fine because [codex::back < 0]
			this->ptr += codec::back(this->ptr);

			return *this;
		}

		// postfix (it--)
		auto operator--(int) -> iterator
		{
			auto temp {*this};
			operator--();
			return temp;
		}

		//|------------|
		//| lhs == rhs |
		//|------------|

		friend auto operator==(const iterator& lhs, const iterator& rhs) -> bool
		{
			return lhs.ptr == rhs.ptr;
		}

		//|------------|
		//| lhs != rhs |
		//|------------|

		friend auto operator!=(const iterator& lhs, const iterator& rhs) -> bool
		{
			return lhs.ptr != rhs.ptr;
		}
	};

	[[nodiscard]]
	friend auto drop(text<T>& to)
	{
		switch (to.mode())
		{
			case tag::SMALL:
			{
				// delete[] to.small;
				break;
			}
			case tag::LARGE:
			{
				delete[] to.large.data;
				break;
			}
		}
	}

public:

	// a -> b
	[[nodiscard]]
	COPY_CALL(text<T>)
	{
		const auto N {from.size()};

		if (dest.capacity() < N)
		{
			dest.capacity(N);
		}
		// copy data
		std::copy
		(
			from.c_str() + 0,
			from.c_str() + N,
			// destination
			dest.c_str()
		);
		// update size
		dest.size(N);
	}

	// a <-> b
	[[nodiscard]]
	SWAP_CALL(text<T>)
	{
		switch (from.mode())
		{
			case tag::SMALL:
			{
				std::swap(from.small, dest.small);
				break;
			}
			case tag::LARGE:
			{
				std::swap(from.large, dest.large);
				break;
			}
		}
	}

	//|---------------|
	//| the rule of 5 |
	//|---------------|

	text() : bytes {0} // clear
	{
		this->bytes[RMB] = MAX << SFT;
		// then...
		assert(this->mode() == tag::SMALL);
	}

	text(const T* ptr) : text() // <- delegation
	{
		if (ptr != nullptr)
		{
			auto len {std::char_traits<T>::length(ptr)};
			// update size
			this->size(len);
			// write data
			std::copy(ptr, ptr + len, this->c_str());
		}
	}

	template<size_t N>
	requires (N <= MAX)
	text(const T (&str)[N]) : text() // <- delegation
	{
		this->size(N);
		// check mode
		assert(this->mode() == tag::SMALL);
		// write data
		std::copy(str, str + N, this->small);
	}

	template<size_t N>
	requires (MAX < N)
	text(const T (&str)[N]) : text() // <- delegation
	{
		this->size(N);
		// check mode
		assert(this->mode() == tag::LARGE);
		// write data
		std::copy(str, str + N, this->large.data);
	}

	text(const size_t size) : text() // <- delegation
	{
		this->capacity(size);
	}

	COPY_CONSTRUCTOR(text) : text() // <- delegation
	{
		if (this != &other)
		{
			copy(other, *this);
		}
	}

	MOVE_CONSTRUCTOR(text) : text() // <- delegation
	{
		if (this != &other)
		{
			swap(other, *this);
		}
	}

	~text()
	{
		drop(*this);
	}

	COPY_ASSIGNMENT(text)
	{
		if (this != &rhs)
		{
			copy(rhs, *this);
		}
		return *this;
	}

	MOVE_ASSIGNMENT(text)
	{
		if (this != &rhs)
		{
			swap(rhs, *this);
		}
		return *this;
	}

	//|-----------------|
	//| member function |
	//|-----------------|

	[[nodiscard]]
	auto c_str() -> T*
	{
		switch (this->mode())
		{
			case tag::SMALL:
			{
				return this->small;
			}
			case tag::LARGE:
			{
				return this->large.data;
			}
		}
		assert(false && "-Wreturn-type");
	}

	[[nodiscard]]
	auto c_str() const -> const T*
	{
		switch (this->mode())
		{
			case tag::SMALL:
			{
				return this->small;
			}
			case tag::LARGE:
			{
				return this->large.data;
			}
		}
		assert(false && "-Wreturn-type");
	}

	class codec
	{
		typedef int8_t width_t;

	public:

		[[nodiscard]] static
		// where : 0 < result
		auto next(const T* ptr) -> width_t;

		[[nodiscard]] static
		// where : result < 0
		auto back(const T* ptr) -> width_t;

		[[nodiscard]] static
		// where : 0 < result
		auto width(const char32_t code) -> width_t;

		[[nodiscard]] static
		void encode(const char32_t in, T* out, width_t width);

		[[nodiscard]] static
		void decode(const T* in, char32_t& out, width_t width);
	};

	class slice
	{
		const T* head;
		const T* tail;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		slice
		(
			decltype(head) head,
			decltype(tail) tail
		)
		: head {head}, tail {tail} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		[[nodiscard]]
		auto to_utf8() const -> text<char8_t>
		{
			text<char8_t> rvalue {this->length() * 4};

			char8_t* ptr {rvalue.c_str()};

			for (const auto code : *this)
			{
				auto width {text<char8_t>::codec::width(code)};
				text<char8_t>::codec::encode(code, ptr, width);
				ptr += width; // move to next code point slot
			}
			// update size
			rvalue.size(ptr - rvalue.c_str());

			return rvalue;
		}

		[[nodiscard]]
		auto to_utf16() const -> text<char16_t>
		{
			text<char16_t> rvalue {this->length() * 2};

			char16_t* ptr {rvalue.c_str()};

			for (const auto code : *this)
			{
				auto width {text<char16_t>::codec::width(code)};
				text<char16_t>::codec::encode(code, ptr, width);
				ptr += width; // move to next code point slot
			}
			// update size
			rvalue.size(ptr - rvalue.c_str());

			return rvalue;
		}

		[[nodiscard]]
		auto to_utf32() const -> text<char32_t>
		{
			text<char32_t> rvalue {this->length() * 1};

			char32_t* ptr {rvalue.c_str()};

			for (const auto code : *this)
			{
				auto width {text<char32_t>::codec::width(code)};
				text<char32_t>::codec::encode(code, ptr, width);
				ptr += width; // move to next code point slot
			}
			// update size
			rvalue.size(ptr - rvalue.c_str());

			return rvalue;
		}

		[[nodiscard]]
		auto substr(const size_t start, const size_t count) const -> slice
		{
			const auto ptr {this->head};
			// UTF-32
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return {ptr + start, ptr + start + count};
			}
			// UTF-8/16
			size_t a {0};
	
			for (size_t i {0}; ptr[a] && i < start; ++i)
			{
				a += codec::next(ptr + a);
			}
			// continue
			size_t b {a};
	
			for (size_t i {0}; ptr[b] && i < count; ++i)
			{
				b += codec::next(ptr + b);
			}
			return {ptr + a, ptr + b};
		}

		[[nodiscard]]
		auto length() const -> size_t
		{
			// UTF-32
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return this->tail - this->head;
			}
			// UTF-8/16
			const auto ptr {this->head};

			size_t i {0};
			size_t j {0};

			for (; ptr[i]; ++j)
			{
				i += codec::next(ptr + i);
			}
			return j; // <- O(N)
		}

		[[nodiscard]]
		auto size() const -> size_t
		{
			return this->tail - this->head;
		}

		//|------------|
		//| lhs == rhs |
		//|------------|

		auto operator==(const slice& rhs) -> bool
		{
			return
			(
				this->head == rhs.head
				&&
				this->tail == rhs.tail
			);
		}

		auto operator==(const text<T>& rhs) -> bool
		{
			if (this->tail - this->head != rhs.size())
			{
				return false;
			}
			// skip a null terminator, thus - 1
			const auto total {(rhs.size() - 1) * sizeof(T)};
			// content equality without null terminator
			return std::memcmp(this->head, rhs.c_str(), total) == 0;
		}
	
		template<size_t N>
		auto operator==(const T (&rhs)[N]) -> bool
		{
			if (this->tail - this->head != N)
			{
				return false;
			}
			// skip a null terminator, thus - 1
			const auto total {(N - 1) * sizeof(T)};
			// content equality without null terminator
			return std::memcmp(this->head, rhs, total) == 0;
		}

		//|------------|
		//| lhs != rhs |
		//|------------|

		auto operator!=(const slice& rhs) -> bool
		{
			return !this->operator==(rhs);
		}

		auto operator!=(const text<T>& rhs) -> bool
		{
			return !this->operator==(rhs);
		}
	
		template<size_t N>
		auto operator!=(const T (&rhs)[N]) -> bool
		{
			return !this->operator==(rhs);
		}

		//|---------------------|
		//| traits::iterable<T> |
		//|---------------------|

		[[nodiscard]]
		auto begin() const -> iterator
		{
			return {this->head};
		}

		[[nodiscard]]
		auto end() const -> iterator
		{
			return {this->tail};
		}

		//|----------------------|
		//| traits::printable<T> |
		//|----------------------|

		friend auto operator<<(std::ostream& os, const slice& str) -> std::ostream&
		{
			for (const auto code : str) { os << code; } return os; // STL support :3
		}
	};

	[[nodiscard]]
	auto to_utf8() const -> text<char8_t>
	{
		text<char8_t> rvalue {this->length() * 4};

		char8_t* ptr {rvalue.c_str()};

		for (const auto code : *this)
		{
			auto width {text<char8_t>::codec::width(code)};
			text<char8_t>::codec::encode(code, ptr, width);
			ptr += width; // move to next code point slot
		}
		// update size
		rvalue.size(ptr - rvalue.c_str());

		return rvalue;
	}

	[[nodiscard]]
	auto to_utf16() const -> text<char16_t>
	{
		text<char16_t> rvalue {this->length() * 2};

		char16_t* ptr {rvalue.c_str()};

		for (const auto code : *this)
		{
			auto width {text<char16_t>::codec::width(code)};
			text<char16_t>::codec::encode(code, ptr, width);
			ptr += width; // move to next code point slot
		}
		// update size
		rvalue.size(ptr - rvalue.c_str());

		return rvalue;
	}

	[[nodiscard]]
	auto to_utf32() const -> text<char32_t>
	{
		text<char32_t> rvalue {this->length() * 1};

		char32_t* ptr {rvalue.c_str()};

		for (const auto code : *this)
		{
			auto width {text<char32_t>::codec::width(code)};
			text<char32_t>::codec::encode(code, ptr, width);
			ptr += width; // move to next code point slot
		}
		// update size
		rvalue.size(ptr - rvalue.c_str());

		return rvalue;
	}

	[[nodiscard]]
	auto substr(const size_t start, const size_t count) const -> slice
	{
		const auto ptr {this->c_str()};
		// UTF-32
		if constexpr (std::is_same_v<T, char32_t>)
		{
			return {ptr + start, ptr + start + count};
		}
		// UTF-8/16
		size_t a {0};

		for (size_t i {0}; ptr[a] && i < start; ++i)
		{
			a += codec::next(ptr + a);
		}
		// continue
		size_t b {a};

		for (size_t i {0}; ptr[b] && i < count; ++i)
		{
			b += codec::next(ptr + b);
		}
		return {ptr + a, ptr + b};
	}

	[[nodiscard]]
	auto length() const -> size_t
	{
		// UTF-32
		if constexpr (std::is_same_v<T, char32_t>)
		{
			return this->size();
		}
		// UTF-8/16
		const auto ptr {this->c_str()};

		size_t i {0};
		size_t j {0};

		for (; ptr[i]; ++j)
		{
			i += codec::next(ptr + i);
		}
		return j; // <- O(N)
	}

	//|------------|
	//| lhs == rhs |
	//|------------|

	auto operator==(const text<T>& rhs) -> bool
	{
		if (this->size() != rhs.size())
		{
			return false;
		}
		// skip a null terminator, thus - 1
		const auto total {(this->size() - 1) * sizeof(T)};
		// content equality without null terminator
		return std::memcmp(this->c_str(), rhs.c_str(), total) == 0;
	}

	auto operator==(const slice& rhs) -> bool
	{
		if (this->size() != rhs.tail - rhs.head)
		{
			return false;
		}
		// skip a null terminator, thus - 1
		const auto total {(this->size() - 1) * sizeof(T)};
		// content equality without null terminator
		return std::memcmp(this->c_str(), rhs.head, total) == 0;
	}

	template<size_t N>
	auto operator==(const T (&rhs)[N]) -> bool
	{
		if (this->size() != N)
		{
			return false;
		}
		// skip a null terminator, thus - 1
		const auto total {(N - 1) * sizeof(T)};
		// content equality without null terminator
		return std::memcmp(this->c_str(), rhs, total) == 0;
	}

	//|------------|
	//| lhs != rhs |
	//|------------|

	auto operator!=(const text<T>& rhs) -> bool
	{
		return !this->operator==(rhs);
	}

	auto operator!=(const slice& rhs) -> bool
	{
		return !this->operator==(rhs);
	}

	template<size_t N>
	auto operator!=(const T (&rhs)[N]) -> bool
	{
		return !this->operator==(rhs);
	}

	//|------------|
	//| lhs += rhs |
	//|------------|

	auto operator+=(const text<T>& rhs) -> decltype(*this)
	{
		const auto total {this->size() + rhs.size()};

		if (this->capacity() < total)
		{
			this->capacity(total);
		}
		auto const N {rhs.size()};
		// copy data
		std::copy
		(
			rhs.c_str() + 0,
			rhs.c_str() + N,
			// destination
			this->c_str() + this->size()
		);
		// update size
		this->size(total);

		return *this;
	}

	auto operator+=(const slice& rhs) -> decltype(*this)
	{
		const auto total {this->size() + rhs.size()};

		if (this->capacity() < total)
		{
			this->capacity(total);
		}
		// copy data
		std::copy
		(
			rhs.head + 0,
			rhs.tail + 0,
			// destination
			this->c_str() + this->size()
		);
		// update size
		this->size(total);

		return *this;
	}

	template<size_t N>
	auto operator+=(const T (&rhs)[N]) -> decltype(*this)
	{
		const auto total {this->size() + N - 1};

		if (this->capacity() < total)
		{
			this->capacity(total);
		}
		// copy data
		std::copy
		(
			rhs + 0,
			rhs + N,
			// destination
			this->c_str() + this->size()
		);
		// update size
		this->size(total);

		return *this;
	}

	//|-----------|
	//| lhs + rhs |
	//|-----------|

	auto operator+(const text<T>& rhs) const -> text<T>
	{
		text<T> rvalue {this->size() + rhs.size()};

		// copy lhs
		rvalue += *this;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	auto operator+(const slice& rhs) const -> text<T>
	{
		text<T> rvalue {this->size() + rhs.size()};

		// copy lhs
		rvalue += *this;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	template<size_t N>
	auto operator+(const T (&rhs)[N]) const -> text<T>
	{
		text<T> rvalue {this->size() + N - 1};

		// copy lhs
		rvalue += *this;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	//|--------|
	//| lhs[N] |
	//|--------|

	auto operator[](const size_t nth) const -> proxy<decltype(*this)>
	{
		return {*this, nth};
	}

	auto operator[](const size_t nth) -> proxy<decltype(*this)>
	{
		return {*this, nth};
	}

	//|---------------------|
	//| traits::iterable<T> |
	//|---------------------|

	[[nodiscard]]
	auto begin() const -> iterator
	{
		return {this->c_str()/* from RE:0 */};
	}

	[[nodiscard]]
	auto end() const -> iterator
	{
		return {this->c_str() + this->size()};
	}

	//|----------------------|
	//| traits::printable<T> |
	//|----------------------|

	friend auto operator<<(std::ostream& os, const text<T>& str) -> std::ostream&
	{
		for (const auto code : str) { os << code; } return os; // STL support :3
	}
};

namespace
{
	constexpr const int8_t TBL[]
	{
		/*|----(0xxx)----|*/
		/*| 0x0          |*/ 1,
		/*| 0x1          |*/ 1,
		/*| 0x2          |*/ 1,
		/*| 0x3          |*/ 1,
		/*| 0x4          |*/ 1,
		/*| 0x5          |*/ 1,
		/*| 0x6          |*/ 1,
		/*| 0x7          |*/ 1,
		/*|----(10xx)----|*/
		/*| 0x8          |*/ 1,
		/*| 0x9          |*/ 1,
		/*| 0xA          |*/ 1,
		/*| 0xB          |*/ 1,
		/*|----(110x)----|*/
		/*| 0xC          |*/ 2,
		/*| 0xD          |*/ 2,
		/*|----(1110)----|*/
		/*| 0xE          |*/ 3,
		/*|----(1111)----|*/
		/*| 0xF          |*/ 4,
		/*|--------------|*/
	};

	[[nodiscard]]
	auto is_lead(char16_t unit) -> bool
	{
		return 0xD800 <= unit && unit <= 0xDBFF;
	}
	
	[[nodiscard]]
	auto is_tail(char16_t unit) -> bool
	{
		return 0xDC00 <= unit && unit <= 0xDFFF;
	}
}

// https://en.wikipedia.org/wiki/UTF-8
typedef text<char8_t> utf8;

static_assert(traits::printable<utf8>);
static_assert(traits::rule_of_5<utf8>);

template<>
auto utf8::codec::next(const char8_t* ptr) -> width_t
{
	//-------------------------------------------//
	return TBL[(((unsigned) *ptr) >> 4) & 0x0F]; //
	//-------------------------------------------//
};

template<>
auto utf8::codec::back(const char8_t* ptr) -> width_t
{
	width_t i {0};
	//-------------------------------------------//
	while ((ptr[i - 1] & 0xC0) == 0x80) { --i; } //
	//-------------------------------------------//
	return i - 1;
};

template<>
auto utf8::codec::width(const char32_t code) -> width_t
{
	const auto N {std::bit_width((unsigned) code)};
	//-------------------------------------------//
	return 1 + (8 <= N) + (12 <= N) + (17 <= N); //
	//-------------------------------------------//
};

template<>
void utf8::codec::encode(const char32_t in, char8_t* out, width_t width)
{
	switch (width)
	{
		case -1:
		case +1:
		{
			out[0] = in;
			break;
		}
		case -2:
		case +2:
		{
			out[0] = 0xC0 | ((in >> 06) & 0x1F);
			out[1] = 0x80 | ((in >> 00) & 0x3F);
			break;
		}
		case -3:
		case +3:
		{
			out[0] = 0xE0 | ((in >> 12) & 0x0F);
			out[1] = 0x80 | ((in >> 06) & 0x3F);
			out[2] = 0x80 | ((in >> 00) & 0x3F);
			break;
		}
		case -4:
		case +4:
		{
			out[0] = 0xF0 | ((in >> 18) & 0x07);
			out[1] = 0x80 | ((in >> 12) & 0x3F);
			out[2] = 0x80 | ((in >> 06) & 0x3F);
			out[3] = 0x80 | ((in >> 00) & 0x3F);
			break;
		}
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
}

template<>
void utf8::codec::decode(const char8_t* in, char32_t& out, width_t width)
{
	switch (width)
	{
		case -1:
		case +1:
		{
			out = in[0];
			break;
		}
		case -2:
		case +2:
		{
			out = 0;
			out |= (in[0] & 0x1F) << 06;
			out |= (in[1] & 0x3F) << 00;
			break;
		}
		case -3:
		case +3:
		{
			out = 0;
			out |= (in[0] & 0x0F) << 12;
			out |= (in[1] & 0x3F) << 06;
			out |= (in[2] & 0x3F) << 00;
			break;
		}
		case -4:
		case +4:
		{
			out = 0;
			out |= (in[0] & 0x07) << 18;
			out |= (in[1] & 0x3F) << 12;
			out |= (in[2] & 0x3F) << 06;
			out |= (in[3] & 0x3F) << 00;
			break;
		}
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
};

// https://en.wikipedia.org/wiki/UTF-16
typedef text<char16_t> utf16;

static_assert(traits::printable<utf16>);
static_assert(traits::rule_of_5<utf16>);

template<>
auto utf16::codec::next(const char16_t* ptr) -> width_t
{
	if (is_lead(ptr[0])) { return +2; }
	if (is_tail(ptr[0])) { return +1; }

	return +1;
};

template<>
auto utf16::codec::back(const char16_t* ptr) -> width_t
{
	if (is_tail(ptr[0])) { return -2; }
	if (is_lead(ptr[0])) { return -1; }

	return -1;
};

template<>
auto utf16::codec::width(const char32_t code) -> width_t
{
	//--------------------------//
	return 1 + (0xFFFF < code); //
	//--------------------------//
};

template<>
void utf16::codec::encode(const char32_t in, char16_t* out, width_t width)
{
	switch (width)
	{
		case -1:
		case +1:
		{
			out[0] = in;
			break;
		}
		case -2:
		case +2:
		{
			const auto code {in - 0x10000};
			out[0] = 0xD800 | (code / 0x400);
			out[1] = 0xDC00 | (code & 0x3FF);
			break;
		}
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
}

template<>
void utf16::codec::decode(const char16_t* in, char32_t& out, width_t width)
{
	switch (width)
	{
		case -1:
		case +1:
		{
			out = in[0];
			break;
		}
		case -2:
		case +2:
		{
			out = 0x10000; // no plus op
			out |= (in[0] - 0xD800) << 10;
			out |= (in[1] - 0xDC00) << 00;
			break;
		}
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
};

// https://en.wikipedia.org/wiki/UTF-32
typedef text<char32_t> utf32;

static_assert(traits::printable<utf32>);
static_assert(traits::rule_of_5<utf32>);

template<>
auto utf32::codec::next(const char32_t* ptr) -> width_t
{
	return +1;
};

template<>
auto utf32::codec::back(const char32_t* ptr) -> width_t
{
	return -1;
};

template<>
auto utf32::codec::width(const char32_t unit) -> width_t
{
	return +1;
};

template<>
void utf32::codec::encode(const char32_t in, char32_t* out, width_t width)
{
	out[0] = in;
}

template<>
void utf32::codec::decode(const char32_t* in, char32_t& out, width_t width)
{
	out = in[0];
};

//|--------------|
//| useful stuff |
//|--------------|

namespace type
{
	template<typename T>
	concept string =
	(
		std::is_same_v<T, utf8>
		||
		std::is_same_v<T, utf16>
		||
		std::is_same_v<T, utf32>
	);
}
