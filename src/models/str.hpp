#pragma once

#include <bit>
#include <vector>
#include <string>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <ostream>
#include <concepts>
#include <algorithm>
#include <type_traits>

#include "utils/ordering.hpp"

#include "traits/printable.hpp"
#include "traits/rule_of_5.hpp"

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
	#define IS_BIG          \
	(                       \
		std::endian::native \
		         !=         \
		std::endian::little \
	)                       \

	enum tag : uint8_t
	{
		SMALL = IS_BIG ? 0b0000000'0 : 0b0'0000000,
		LARGE = IS_BIG ? 0b0000000'1 : 0b1'0000000,
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

		auto begin() -> T*
		{
			return this->data; // from start
		}

		auto end() -> T*
		{
			return this->data + this->size;
		}

		auto begin() const -> const T*
		{
			return this->data; // from start
		}

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

	// getter
	auto size() const -> size_t
	{
		switch (this->mode())
		{
			case tag::SMALL:
			{
				//|------------|    |-[little endian]-|
				//| 0b0XXXXXXX | -> | no need to skip |
				//|------------|    |-----------------|

				//|------------|    |---[big endian]---|
				//| 0bXXXXXXX0 | -> | skip right 1 bit |
				//|------------|    |------------------|

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

	// setter
	auto size(const size_t value)
	{
		if (this->capacity() < value + 1)
		{
			allocate:
			auto data {new T[value + 1]};

			switch (this->mode())
			{
				// S -> L
				case tag::SMALL:
				{
					std::ranges::copy(this->small, data);
					// delete[] this->small;
					break;
				}
				// L -> L
				case tag::LARGE:
				{
					std::ranges::copy(this->large, data);
					delete[] this->large.data;
					break;
				}
			}
			this->large.data = data;
			this->large.size = value + 0;
			this->large.capacity = value + 1;
			this->large.metadata = tag::LARGE;
		}
		else if (value < this->capacity())
		{
			switch (this->mode())
			{
				// L -> L
				case tag::LARGE:
				{
					this->large.size = value;
					// terminate
					this->large[value] = '\0';
					break;
				}
				// S -> S
				case tag::SMALL:
				{
					assert(value <= RMB);

					auto slot {MAX - value};

					//|------------|    |-[little endian]-|
					//| 0b0XXXXXXX | -> | no need to skip |
					//|------------|    |-----------------|

					//|------------|    |---[big endian]---|
					//| 0bXXXXXXX0 | -> | skip right 1 bit |
					//|------------|    |------------------|

					this->bytes[RMB] = slot << SFT;
					// terminate
					this->small[value] = '\0';
					break;
				}
			}
		}
		assert(this->size() < this->capacity());
	}

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
					std::ranges::copy(this->small, data);
					// delete[] this->small;
					break;
				}
				// L -> L
				case tag::LARGE:
				{
					std::ranges::copy(this->large, data);
					delete[] this->large.data;
					break;
				}
			}
			this->large.data = data;
			this->large.size = size;
			this->large.capacity = value;
			this->large.metadata = tag::LARGE;
		}
		else if (value < this->capacity())
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
					// ¯\_(ツ)_/¯
					break;
				}
			}
		}
		assert(this->size() < this->capacity());
	}

private:

	template
	<
		typename S
	>
	requires
	(
		std::is_same_v<S, text<T>&>
		||
		std::is_same_v<S, const text<T>&>
	)
	class proxy
	{
		S src;
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
			!std::is_const_v<std::remove_reference_t<S>>
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

							if (this->src.capacity() < new_l + 1)
							{
								this->src.capacity(new_l * 2);
							}
							// copy right->left
							std::ranges::copy_backward
							(
								ptr + i + b,
								ptr + old_l,
								ptr + i + a
							);
							this->src.size(new_l);
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

							// copy left->right
							std::ranges::copy
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

public:

	// a -> b
	COPY_CALL(text<T>)
	{
		const auto N {from.size()};

		if (dest.capacity() < N + 1)
		{
			dest.capacity(N + 1);
		}
		// copy data
		std::ranges::copy
		(
			from.c_str() + 0,
			from.c_str() + N,
			// destination
			dest.c_str() + 0
		);
		// copy size
		dest.size(N);
	}

	// a <-> b
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

	text(const size_t size) : text()
	{
		this->capacity(size);
	}

	template<size_t N>
	requires (N <= MAX)
	text(const T (&str)[N]) : text()
	{
		this->size(N);
		// check mode
		assert(this->mode() == tag::SMALL);
		// write data
		std::ranges::copy(str, str + N, this->small);
	}

	template<size_t N>
	requires (MAX < N)
	text(const T (&str)[N]) : text()
	{
		this->size(N);
		// check mode
		assert(this->mode() == tag::LARGE);
		// write data
		std::ranges::copy(str, str + N, this->large.data);
	}

	text(const T* ptr) : text()
	{
		if (ptr != nullptr)
		{
			const auto N {std::char_traits<T>::length(ptr)};
			// copy meta
			this->size(N);
			// copy data
			std::ranges::copy(ptr, ptr + N + 1, this->c_str());
		}
	}

	COPY_CONSTRUCTOR(text) : text()
	{
		if (this != &other)
		{
			copy(other, *this);
		}
	}

	MOVE_CONSTRUCTOR(text) : text()
	{
		if (this != &other)
		{
			swap(other, *this);
		}
	}

	~text()
	{
		if (this->mode() == tag::LARGE)
		{
			delete[] this->large.data;
		}
		// no need to delete in SSO mode
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

	class codec
	{
		typedef int8_t width_t;

	public:

		static inline
		// where : 0 < result
		auto next(const T* ptr) -> width_t;

		static inline
		// where : result < 0
		auto back(const T* ptr) -> width_t;

		static inline
		// where : 0 < result
		auto width(const char32_t code) -> width_t;

		static inline
		void encode(const char32_t in, T* out, width_t width);

		static inline
		void decode(const T* in, char32_t& out, width_t width);
	};

	//|-----------------|
	//| member function |
	//|-----------------|

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

	//|--------------|
	//| lhs.length() |
	//|--------------|

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

	class slice
	{
		friend text;

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

		auto size() const -> size_t
		{
			return this->tail - this->head;
		}

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

		auto to_utf8() const -> text<char8_t>
		{
			text<char8_t> rvalue {this->size() + 1};

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

		auto to_utf16() const -> text<char16_t>
		{
			text<char16_t> rvalue {this->size() + 1};

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

		auto to_utf32() const -> text<char32_t>
		{
			text<char32_t> rvalue {this->size() + 1};

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

		//|------------|
		//| lhs.find() |
		//|------------|

		template<typename U>
		auto find(const text<U>& str, const size_t offset = 0) const -> size_t
		{
			const T* ptr {this->head};

			size_t nth {0};
			char32_t out {0};

			for (; nth < offset; ++nth)
			{
				ptr += codec::next(ptr);
			}

			while (ptr < this->tail)
			{
				auto width {codec::next(ptr)};
				codec::decode(ptr, out, width);

				if (out == str[0]) // match!
				{
					const T* temp {ptr};

					size_t i {0};

					// check match
					for (const auto code : str)
					{
						if (this->tail <= temp)
						{
							break;
						}
						// hopefully no segfault
						auto width {codec::next(temp)};
						codec::decode(temp, out, width);

						if (out != code)
						{
							break;
						}
						// increase both temp & i
						temp += width; ++i;
					}
					// full match
					if (i == str.length())
					{
						return nth - offset;
					}
				}
				// increase both ptr & nth
				ptr += width; ++nth;
			}
			return SIZE_MAX;
		}

		template<size_t N>
		auto find(const T (&str)[N], const size_t offset = 0) const -> size_t
		{
			const T* ptr {this->head};

			size_t nth {0};
			char32_t out {0};

			for (; nth < offset; ++nth)
			{
				ptr += codec::next(ptr);
			}

			while (ptr < this->tail)
			{
				auto width {codec::next(ptr)};
				codec::decode(ptr, out, width);

				if (out == str[0]) // match!
				{
					const T* temp {ptr};

					size_t i {0};

					// check match
					for (const auto code : str)
					{
						if (this->tail <= temp)
						{
							break;
						}
						// hopefully no segfault
						auto width {codec::next(temp)};
						codec::decode(temp, out, width);

						if (out != code)
						{
							break;
						}
						// increase both temp & i
						temp += width; ++i;
					}
					// full match
					if (i == N - 1)
					{
						return nth - offset;
					}
				}
				// increase both ptr & nth
				ptr += width; ++nth;
			}
			return SIZE_MAX;
		}

		auto find(const char32_t code, const size_t offset = 0) const -> size_t
		{
			const T* ptr {this->head};

			size_t nth {0};
			char32_t out {0};

			for (; nth < offset; ++nth)
			{
				ptr += codec::next(ptr);
			}

			while (ptr < this->tail)
			{
				auto width {codec::next(ptr)};
				codec::decode(ptr, out, width);

				if (out == code) // match!
				{
					return nth - offset;
				}
				ptr += width; ++nth;
			}
			return SIZE_MAX;
		}

		template<size_t N>
		// converting constructor
		auto find(const char8_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char8_t>)
		{
			return this->find(text<char8_t>{str}, offset);
		}

		template<size_t N>
		// converting constructor
		auto find(const char16_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char16_t>)
		{
			return this->find(text<char16_t>{str}, offset);
		}

		template<size_t N>
		// converting constructor
		auto find(const char32_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char32_t>)
		{
			return this->find(text<char32_t>{str}, offset);
		}

		//|-------------|
		//| lhs.split() |
		//|-------------|

		template<typename U>
		auto split(const text<U>& str) const -> std::vector<slice>
		{
			if constexpr (std::is_same_v<T, U>)
			{
				std::vector<slice> result;

				const T* head {this->head};
				const T* tail {this->head};
				//-------------------------//
				const auto N {str.size()}; //
				//-------------------------//

				while (tail < this->tail)
				{
					if (head + N - 1 < this->tail)
					{
						size_t i {0};

						// check match
						for (; i < N - 1; ++i)
						{
							if (tail[i] != str[i])
							{
								break;
							}
						}
						// full match
						if (i == N - 1)
						{
							result.emplace_back(head, tail);
							// move start to the next
							head = tail = tail + i;
						}
						++tail;
					}
					else
					{
						break;
					}
				}
				// rest of the slice
				if (head <= this->tail)
				{
					result.emplace_back(head, this->tail);
				}
				return result;
			}
			else // if (!std::is_same_v<T, U>)
			{
				std::vector<slice> result;

				char32_t out {'\0'};

				const T* head {this->head};
				const T* tail {this->head};

				while (tail < this->tail)
				{
					auto width {codec::next(tail)};
					codec::decode(tail, out, width);

					if (out == str[0])
					{
						const T* ptr {tail};

						size_t i {0};

						// check match
						for (const auto code : str)
						{
							if (this->tail <= ptr)
							{
								break;
							}
							// hopefully no segfault
							auto width {codec::next(ptr)};
							codec::decode(ptr, out, width);

							if (out != code)
							{
								break;
							}
							// increase both ptr & i
							ptr += width; ++i;
						}
						// full match
						if (i == str.length())
						{
							result.emplace_back(head, tail);
							// move start to the next
							head = tail = ptr;
							// avoid goto?
							continue;
						}
					}
					tail += width;
				}
				// rest of the slice
				if (head <= this->tail)
				{
					result.emplace_back(head, this->tail);
				}
				return result;
			}
		}

		template<size_t N>
		auto split(const T (&str)[N]) const -> std::vector<slice>
		{
			std::vector<slice> result;

			const T* head {this->head};
			const T* tail {this->head};

			while (tail < this->tail)
			{
				if (head + N - 1 < this->tail)
				{
					size_t i {0};

					// check match
					for (; i < N - 1; ++i)
					{
						if (str[i] != tail[i])
						{
							break;
						}
					}
					// full match
					if (i == N - 1)
					{
						result.emplace_back(head, tail);
						// move start to the next
						head = tail = tail + i;
					}
					++tail;
				}
				else
				{
					break;
				}
			}
			// rest of the slice
			if (head <= this->tail)
			{
				result.emplace_back(head, this->tail);
			}
			return result;
		}

		auto split(const char32_t code) const -> std::vector<slice>
		{
			std::vector<slice> result;

			char32_t out {'\0'};

			const T* head {this->head};
			const T* tail {this->head};

			while (tail < this->tail)
			{
				auto width {codec::next(tail)};
				codec::decode(tail, out, width);

				if (out == code)
				{
					result.emplace_back(head, tail);
					// move start to the next
					head = tail + width;
				}
				// move point to the next
				tail += width;
			}
			// rest of the slice
			if (head <= this->tail)
			{
				result.emplace_back(head, this->tail);
			}
			return result;
		}

		template<size_t N>
		// converting constructor
		auto split(const char8_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char8_t>)
		{
			return this->split(text<char8_t>{str});
		}

		template<size_t N>
		// converting constructor
		auto split(const char16_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char16_t>)
		{
			return this->split(text<char16_t>{str});
		}

		template<size_t N>
		// converting constructor
		auto split(const char32_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char32_t>)
		{
			return this->split(text<char32_t>{str});
		}

		//|--------------|
		//| lhs.substr() |
		//|--------------|

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

		//|-----------|
		//| lhs < rhs |
		//|-----------|

		friend auto operator<(const slice& lhs, const slice& rhs) -> bool
		{
			const auto len {std::min
			(lhs.size(), rhs.size())};

			size_t i {0};

			const auto foo {lhs.head};
			const auto bar {rhs.head};

			for (; i < len; ++i)
			{
				if (foo[i] != bar[i])
				{
					return foo[i] < bar[i];
				}
			}
			return lhs.size() < rhs.size();
		}

		//|--------|
		//| lhs[M] |
		//|--------|

		auto operator[](const size_t nth) const -> char32_t
		{
			auto ptr {this->head};

			size_t i {0};
			size_t j {0};

			if constexpr (std::is_same_v<T, char32_t>)
			{
				i = nth;
				j = nth;

				if (nth < this->size())
				{
					goto for_utf32; // <- O(1)
				}
				// no exception
				return U'\0';
			}

			for (; ptr[i]; ++j)
			{
				if (j == nth)
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

		//|------------|
		//| lhs == rhs |
		//|------------|

		friend auto operator==(const slice& lhs, const slice& rhs) -> bool
		{
			return
			(
				lhs.head == rhs.head
				&&
				lhs.tail == rhs.tail
			);
		}

		template<typename U>
		friend auto operator==(const slice& lhs, const text<U>& rhs) -> bool
		{
			if constexpr (std::is_same_v<T, U>)
			{
				if (lhs.tail - lhs->head != rhs.size())
				{
					return false;
				}
				const auto total {rhs.size() * sizeof(T)};
				// content equality without null terminator
				return std::memcmp(lhs.head, rhs.c_str(), total) == 0;
			}
			else // if (!std::is_same_v<T, U>)
			{
				auto l_b {lhs.begin()}; auto l_e {lhs.end()};
				auto r_b {rhs.begin()}; auto r_e {rhs.end()};

				for (;l_b != l_e || r_b != r_e;)
				{
					if (*l_b != *r_b)
					{
						return false;
					}
					++l_b; // next..!
					++r_b; // next..!
				}
				return l_b == l_e && r_b == r_e;
			}
		}

		template<size_t N>
		friend auto operator==(const slice& lhs, const T (&rhs)[N]) -> bool
		{
			if (lhs.tail - lhs.head != N)
			{
				return false;
			}
			// skip a null terminator, thus - 1
			const auto total {(N - 1) * sizeof(T)};
			// content equality without null terminator
			return std::memcmp(lhs.head, rhs, total) == 0;
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator==(const T (&lhs)[N], const slice& rhs) -> bool
		{
			return rhs == lhs;
		}

		template<size_t N>
		// converting constructor
		friend auto operator==(const slice& lhs, const char8_t (&rhs)[N]) -> bool requires (!std::is_same_v<T, char8_t>)
		{
			return lhs == text<char8_t>{rhs};
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator==(const char8_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char8_t>)
		{
			return rhs == lhs;
		}

		template<size_t N>
		// converting constructor
		friend auto operator==(const slice& lhs, const char16_t (&rhs)[N]) -> bool requires (!std::is_same_v<T, char16_t>)
		{
			return lhs == text<char16_t>{rhs};
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator==(const char16_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char16_t>)
		{
			return rhs == lhs;
		}

		template<size_t N>
		// converting constructor
		friend auto operator==(const slice& lhs, const char32_t (&rhs)[N]) -> bool requires (!std::is_same_v<T, char32_t>)
		{
			return lhs == text<char32_t>{rhs};
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator==(const char32_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char32_t>)
		{
			return rhs == lhs;
		}

		//|------------|
		//| lhs != rhs |
		//|------------|

		friend auto operator!=(const slice& lhs, const slice& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		template<typename U>
		friend auto operator!=(const slice& lhs, const text<U>& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		friend auto operator!=(const slice& lhs, const T (&rhs)[N]) -> bool
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		friend auto operator!=(const T (&lhs)[N], const slice& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// converting constructor
		friend auto operator!=(const slice& lhs, const char8_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char8_t>)
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator!=(const char8_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char8_t>)
		{
			return rhs != lhs;
		}

		template<size_t N>
		// converting constructor
		friend auto operator!=(const slice& lhs, const char16_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char16_t>)
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator!=(const char16_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char16_t>)
		{
			return rhs != lhs;
		}

		template<size_t N>
		// converting constructor
		friend auto operator!=(const slice& lhs, const char32_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char32_t>)
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// reverse op overloading
		friend auto operator!=(const char32_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char32_t>)
		{
			return rhs != lhs;
		}

		//|---------------------|
		//| traits::iterable<T> |
		//|---------------------|

		auto begin() const -> iterator
		{
			return {this->head};
		}

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

	//|----------------------|
	//| from slice to string |
	//|----------------------|

	template
	<
		typename slice_t
	>
	requires requires (slice_t str)
	{
		!std::is_same_v<slice_t, text<char8_t>>;
		!std::is_same_v<slice_t, text<char16_t>>;
		!std::is_same_v<slice_t, text<char32_t>>;

		{ str.to_utf8() } -> std::same_as<text<char8_t>>;
		{ str.to_utf16() } -> std::same_as<text<char16_t>>;
		{ str.to_utf32() } -> std::same_as<text<char32_t>>;
	}
	text(const slice_t& str) : text()
	{
		if constexpr (std::is_same_v<T, char8_t>)
		{
			*this = str.to_utf8();
		}
		if constexpr (std::is_same_v<T, char16_t>)
		{
			*this = str.to_utf16();
		}
		if constexpr (std::is_same_v<T, char32_t>)
		{
			*this = str.to_utf32(); 
		}
	}

	template
	<
		typename slice_t
	>
	requires requires (slice_t str)
	{
		!std::is_same_v<slice_t, text<char8_t>>;
		!std::is_same_v<slice_t, text<char16_t>>;
		!std::is_same_v<slice_t, text<char32_t>>;

		{ str.to_utf8() } -> std::same_as<text<char8_t>>;
		{ str.to_utf16() } -> std::same_as<text<char16_t>>;
		{ str.to_utf32() } -> std::same_as<text<char32_t>>;
	}
	auto operator=(const slice_t& rhs) -> text&
	{
		if constexpr (std::is_same_v<T, char8_t>)
		{
			*this = rhs.to_utf8();
		}
		if constexpr (std::is_same_v<T, char16_t>)
		{
			*this = rhs.to_utf16();
		}
		if constexpr (std::is_same_v<T, char32_t>)
		{
			*this = rhs.to_utf32();
		}
		return *this;
	}

	template
	<
		typename slice_t
	>
	requires requires (slice_t str)
	{
		!std::is_same_v<slice_t, text<char8_t>>;
		!std::is_same_v<slice_t, text<char16_t>>;
		!std::is_same_v<slice_t, text<char32_t>>;

		{ str.to_utf8() } -> std::same_as<text<char8_t>>;
		{ str.to_utf16() } -> std::same_as<text<char16_t>>;
		{ str.to_utf32() } -> std::same_as<text<char32_t>>;
	}
	auto operator=(const slice_t&& rhs) -> text&
	{
		if constexpr (std::is_same_v<T, char8_t>)
		{
			*this = rhs.to_utf8();
		}
		if constexpr (std::is_same_v<T, char16_t>)
		{
			*this = rhs.to_utf16();
		}
		if constexpr (std::is_same_v<T, char32_t>)
		{
			*this = rhs.to_utf32();
		}
		return *this;
	}

	//|--------------------------|
	//| utf8 <-> utf16 <-> utf32 |
	//|--------------------------|

	auto to_utf8() const -> text<char8_t>
	{
		if constexpr (std::is_same_v<T, char8_t>)
		{
			return text<char8_t> {*this};
		}
		text<char8_t> rvalue {this->size()};

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

	auto to_utf16() const -> text<char16_t>
	{
		if constexpr (std::is_same_v<T, char16_t>)
		{
			return text<char16_t> {*this};
		}
		text<char16_t> rvalue {this->size()};

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

	auto to_utf32() const -> text<char32_t>
	{
		if constexpr (std::is_same_v<T, char32_t>)
		{
			return text<char32_t> {*this};
		}
		text<char32_t> rvalue {this->size()};

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

	//|------------|
	//| lhs.find() |
	//|------------|

	template<typename U>
	auto find(const text<U>& str, const size_t offset = 0) const -> size_t
	{
		//----------------------------------------------//
		const auto LAST {this->c_str() + this->size()}; //
		//----------------------------------------------//
		const T* ptr {this->c_str()};

		size_t nth {0};
		char32_t out {0};

		for (; nth < offset; ++nth)
		{
			ptr += codec::next(ptr);
		}

		while (ptr < LAST)
		{
			auto width {codec::next(ptr)};
			codec::decode(ptr, out, width);

			if (out == str[0]) // match!
			{
				const T* temp {ptr};

				size_t i {0};

				// check match
				for (const auto code : str)
				{
					if (LAST <= temp)
					{
						break;
					}
					// hopefully no segfault
					auto width {codec::next(temp)};
					codec::decode(temp, out, width);

					if (out != code)
					{
						break;
					}
					// increase both temp & i
					temp += width; ++i;
				}
				// full match
				if (i == str.length())
				{
					return nth - offset;
				}
			}
			// increase both ptr & nth
			ptr += width; ++nth;
		}
		return SIZE_MAX;
	}

	template<size_t N>
	auto find(const T (&str)[N], const size_t offset = 0) const -> size_t
	{
		//----------------------------------------------//
		const auto LAST {this->c_str() + this->size()}; //
		//----------------------------------------------//
		const T* ptr {this->c_str()};

		size_t nth {0};
		char32_t out {0};

		for (; nth < offset; ++nth)
		{
			ptr += codec::next(ptr);
		}

		while (ptr < LAST)
		{
			auto width {codec::next(ptr)};
			codec::decode(ptr, out, width);

			if (out == str[0]) // match!
			{
				const T* temp {ptr};

				size_t i {0};

				// check match
				for (const auto code : str)
				{
					if (LAST <= temp)
					{
						break;
					}
					// hopefully no segfault
					auto width {codec::next(temp)};
					codec::decode(temp, out, width);

					if (out != code)
					{
						break;
					}
					// increase both temp & i
					temp += width; ++i;
				}
				// full match
				if (i == str.length())
				{
					return nth - offset;
				}
			}
			// increase both ptr & nth
			ptr += width; ++nth;
		}
		return SIZE_MAX;
	}

	auto find(const char32_t code, const size_t offset = 0) const -> size_t
	{
		//----------------------------------------------//
		const auto LAST {this->c_str() + this->size()}; //
		//----------------------------------------------//
		const T* ptr {this->c_str()};

		size_t nth {0};
		char32_t out {0};

		for (; nth < offset; ++nth)
		{
			ptr += codec::next(ptr);
		}

		while (ptr < LAST)
		{
			auto width {codec::next(ptr)};
			codec::decode(ptr, out, width);

			if (out == code) // match!
			{
				return nth - offset;
			}
			ptr += width; ++nth;
		}
		return SIZE_MAX;
	}

	template<size_t N>
	// converting constructor
	auto find(const char8_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return this->find(text<char8_t>{str}, offset);
	}

	template<size_t N>
	// converting constructor
	auto find(const char16_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return this->find(text<char16_t>{str}, offset);
	}

	template<size_t N>
	// converting constructor
	auto find(const char32_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return this->find(text<char32_t>{str}, offset);
	}

	//|-------------|
	//| lhs.split() |
	//|-------------|

	template<typename U>
	auto split(const text<U>& str) const -> std::vector<slice>
	{
		if constexpr (std::is_same_v<T, U>)
		{
			//--------------------------------------------//
			const T* LAST {this->c_str() + this->size()}; //
			//--------------------------------------------//
			std::vector<slice> result;

			const T* head {this->c_str()};
			const T* tail {this->c_str()};
			//-------------------------//
			const auto N {str.size()}; //
			//-------------------------//

			while (tail < LAST)
			{
				if (head + N - 1 < LAST)
				{
					size_t i {0};

					// check match
					for (const auto code : str)
					{
						if (tail[i] != code)
						{
							break;
						}
						++i;
					}
					// full match
					if (i == N - 1)
					{
						result.emplace_back(head, tail);
						// move start to the next
						head = tail = tail + i;
					}
					++tail;
				}
				else
				{
					break;
				}
			}
			// rest of the slice
			if (head <= LAST)
			{
				result.emplace_back(head, LAST);
			}
			return result;
		}
		else // if (!std::is_same_v<T, U>)
		{
			//--------------------------------------------//
			const T* LAST {this->c_str() + this->size()}; //
			//--------------------------------------------//
			std::vector<slice> result;

			char32_t out {'\0'};

			const T* head {this->c_str()};
			const T* tail {this->c_str()};

			while (tail < LAST)
			{
				auto width {codec::next(tail)};
				codec::decode(tail, out, width);

				if (out == str[0])
				{
					const T* ptr {tail};

					size_t i {0};

					// check match
					for (const auto code : str)
					{
						if (LAST <= ptr)
						{
							break;
						}
						// hopefully no segfault
						auto width {codec::next(ptr)};
						codec::decode(ptr, out, width);

						if (out != code)
						{
							break;
						}
						// increase both ptr & i
						ptr += width; ++i;
					}
					// full match
					if (i == str.length())
					{
						result.emplace_back(head, tail);
						// move start to the next
						head = tail = ptr;
						// avoid goto?
						continue;
					}
				}
				tail += width;
			}
			// rest of the slice
			if (head <= LAST)
			{
				result.emplace_back(head, LAST);
			}
			return result;
		}
	}

	template<size_t N>
	auto split(const T (&str)[N]) const -> std::vector<slice>
	{
		//--------------------------------------------//
		const T* LAST {this->c_str() + this->size()}; //
		//--------------------------------------------//
		std::vector<slice> result;

		const T* head {this->c_str()};
		const T* tail {this->c_str()};

		while (tail < LAST)
		{
			if (head + N - 1 < LAST)
			{
				size_t i {0};

				// check match
				for (; i < N - 1; ++i)
				{
					if (str[i] != tail[i])
					{
						break;
					}
				}
				// full match
				if (i == N - 1)
				{
					result.emplace_back(head, tail);
					// move start to the next
					head = tail = tail + i;
				}
				++tail;
			}
			else
			{
				break;
			}
		}
		// rest of the slice
		if (head <= LAST)
		{
			result.emplace_back(head, LAST);
		}
		return result;
	}

	auto split(const char32_t code) const -> std::vector<slice>
	{
		//--------------------------------------------//
		const T* LAST {this->c_str() + this->size()}; //
		//--------------------------------------------//
		std::vector<slice> result;

		char32_t out {'\0'};

		const T* head {this->c_str()};
		const T* tail {this->c_str()};

		while (tail < LAST)
		{
			auto width {codec::next(tail)};
			codec::decode(tail, out, width);

			if (out == code)
			{
				result.emplace_back(head, tail);
				// move start to the next
				head = tail + width;
			}
			// move point to the next
			tail += width;
		}
		// rest of the slice
		if (head <= LAST)
		{
			result.emplace_back(head, LAST);
		}
		return result;
	}

	template<size_t N>
	// converting constructor
	auto split(const char8_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return this->split(text<char8_t>{str});
	}

	template<size_t N>
	// converting constructor
	auto split(const char16_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return this->split(text<char16_t>{str});
	}

	template<size_t N>
	// converting constructor
	auto split(const char32_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return this->split(text<char32_t>{str});
	}

	//|--------------|
	//| lhs.substr() |
	//|--------------|

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

	//|-----------|
	//| lhs < rhs |
	//|-----------|

	friend auto operator<(const text<T>& lhs, const text<T>& rhs) -> bool
	{
		const auto len {std::min
		(lhs.size(), rhs.size())};

		size_t i {0};

		const auto foo {lhs.c_str()};
		const auto bar {rhs.c_str()};

		for (; i < len; ++i)
		{
			if (foo[i] != bar[i])
			{
				return foo[i] < bar[i];
			}
		}
		return lhs.size() < rhs.size();
	}

	class builder
	{
		// fragments of source
		std::vector<text<T>> atom;
		// arguments to concat
		std::vector<text<T>> args;

		auto full() const -> bool
		{
			return // until N = N' + 1
			(
				this->atom.size() + 0
				==
				this->args.size() + 1
			);
		}

	public:

		builder(const slice& str)
		{
			for (auto& _ : str.split(u8"%s"))
			{
				if constexpr (std::same_as<T, char8_t>)
				{
					this->atom.emplace_back(_.to_utf8());
				}
				if constexpr (std::same_as<T, char16_t>)
				{
					this->atom.emplace_back(_.to_utf16());
				}
				if constexpr (std::same_as<T, char32_t>)
				{
					this->atom.emplace_back(_.to_utf32());
				}
			}
		}

		builder(const text<T>& str)
		{
			for (auto& _ : str.split(u8"%s"))
			{
				if constexpr (std::same_as<T, char8_t>)
				{
					this->atom.emplace_back(_.to_utf8());
				}
				if constexpr (std::same_as<T, char16_t>)
				{
					this->atom.emplace_back(_.to_utf16());
				}
				if constexpr (std::same_as<T, char32_t>)
				{
					this->atom.emplace_back(_.to_utf32());
				}
			}
		}

		//|-----------------|
		//| member function |
		//|-----------------|

		operator text<T>()&&
		{
			while (!this->full())
			{
				this->args.emplace_back(u8"%s");
			}
			// allocate str
			text<T> result
			{
				[&]
				{
					size_t impl {0};

					for (auto& _ : this->atom) { impl += _.size(); }
					for (auto& _ : this->args) { impl += _.size(); }

					return impl + 1;
				}
				()
			};
			// mix and join
			for (size_t i {0}; i < args.size(); ++i)
			{
				result += this->atom[i]; // write
				result += this->args[i]; // write
			}
			// last fragment
			result += this->atom.back();

			return result;
		}

		//|-----------|
		//| lhs | rhs |
		//|-----------|

		auto operator|(const text<T>& rhs)&& -> builder&
		{
			if (!this->full())
			{
				this->args.emplace_back(rhs);
			}
			return *this;
		}

		auto operator|(const slice& rhs)&& -> builder&
		{
			if (!this->full())
			{
				this->args.emplace_back(rhs);
			}
			return *this;
		}

		template<size_t N>
		auto operator|(const T (&rhs)[N])&& -> builder&
		{
			if (!this->full())
			{
				this->args.emplace_back(rhs);
			}
			return *this;
		}

		//|-------------------|
		//| fundamental types |
		//|-------------------|

		auto operator|(const bool rhs)&& -> builder&
		{
			return *this | (rhs ? u8"true" : u8"false");
		}

		// auto operator|(const int64_t rhs) && -> builder&
		// {
		// 	return *this | utils::itoa(rhs);
		// }
	
		// auto operator|(const uint64_t rhs) && -> builder&
		// {
		// 	return *this | utils::utoa(rhs);
		// }
	};

	//|-----------|
	//| lhs | rhs |
	//|-----------|

	template<typename U>
	friend auto operator|(const text<T> lhs, const text<U>& rhs) -> builder
	{
		return builder {lhs} | rhs;
	}

	template<size_t N>
	friend auto operator|(const text<T>& lhs, const slice& rhs) -> builder
	{
		return builder {lhs} | rhs;
	}

	template<size_t N>
	friend auto operator|(const slice& lhs, const text<T>& rhs) -> builder
	{
		return builder {lhs} | rhs;
	}

	template<size_t N>
	friend auto operator|(const text<T>& lhs, const T (&rhs)[N]) -> builder
	{
		return builder {lhs} | rhs;
	}

	template<size_t N>
	friend auto operator|(const T (&lhs)[N], const text<T>& rhs) -> builder
	{
		return builder {lhs} | rhs;
	}

	//|------------|
	//| lhs += rhs |
	//|------------|

	/*************<ptr & offset>**************/
	#define BUFFER this->c_str() + this->size()
	/*****************************************/

	template<typename U>
	auto operator+=(const text<U>& rhs) -> text<T>
	{
		const auto total {this->size() + rhs.size()};

		if constexpr (std::is_same_v<T, U>)
		{
			if (this->capacity() < total + 1)
			{
				this->capacity(total + 1);
			}
			assert(BUFFER != nullptr);

			auto const N {rhs.size()};

			std::ranges::copy
			(
				rhs.c_str() + 0,
				rhs.c_str() + N,
				// dest
				BUFFER
			);

			this->size(total);

			return *this;
		}
		else // if (!std::is_same_v<T, U>)
		{
			if constexpr (std::is_same_v<T, char8_t>)
			{
				return *this += rhs.to_utf8();
			}
			if constexpr (std::is_same_v<T, char16_t>)
			{
				return *this += rhs.to_utf16();
			}
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return *this += rhs.to_utf32();
			}
		}
	}

	auto operator+=(const slice& rhs) -> text<T>
	{
		const auto total {this->size() + rhs.size()};

		if (this->capacity() < total + 1)
		{
			this->capacity(total + 1);
		}
		assert(BUFFER != nullptr);

		std::ranges::copy
		(
			rhs.head + 0,
			rhs.tail + 0,
			// dest
			BUFFER
		);

		this->size(total);

		return *this;
	}

	template<size_t N>
	auto operator+=(const T (&rhs)[N]) -> text<T>
	{
		const auto total {this->size() + N - 1};

		if (this->capacity() < total + 1)
		{
			this->capacity(total + 1);
		}
		assert(BUFFER != nullptr);

		std::ranges::copy
		(
			rhs + 0,
			rhs + N,
			// dest
			BUFFER
		);

		this->size(total);

		return *this;
	}

	#undef BUFFER

	//|-----------|
	//| lhs + rhs |
	//|-----------|

	template<typename U>
	friend auto operator+(const text<T> lhs, const text<U>& rhs) -> text<T>
	{
		text<T> rvalue {lhs.size() + rhs.size()};

		// copy lhs
		rvalue += lhs;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	friend auto operator+(const text<T> lhs, const slice& rhs) -> text<T>
	{
		text<T> rvalue {lhs.size() + rhs.size()};

		// copy lhs
		rvalue += lhs;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	// reverse op overloading
	friend auto operator+(const slice& lhs, const text<T> rhs) -> text<T>
	{
		text<T> rvalue {lhs.size() + rhs.size()};

		// copy lhs
		rvalue += lhs;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	template<size_t N>
	friend auto operator+(const text<T> lhs, const T (&rhs)[N]) -> text<T>
	{
		text<T> rvalue {lhs.size() + N - 1};

		// copy lhs
		rvalue += lhs;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator+(const T (&lhs)[N], const text<T> rhs) -> text<T>
	{
		text<T> rvalue {N - 1 + rhs.size()};

		// copy lhs
		rvalue += lhs;
		// copy rhs
		rvalue += rhs;

		return rvalue;
	}

	//|------------|
	//| lhs == rhs |
	//|------------|

	template<typename U>
	friend auto operator==(const text<T>& lhs, const text<U>& rhs) -> bool
	{
		if constexpr (std::is_same_v<T, U>)
		{
			if (lhs.size() != rhs.size())
			{
				return false;
			}
			const auto total {lhs.size() * sizeof(T)};
			// content equality without null terminator
			return std::memcmp(lhs.c_str(), rhs.c_str(), total) == 0;
		}
		else // if (!std::is_same_v<T, U>)
		{
			auto l_b {lhs.begin()}; auto l_e {lhs.end()};
			auto r_b {rhs.begin()}; auto r_e {rhs.end()};

			for (;l_b != l_e || r_b != r_e;)
			{
				if (*l_b != *r_b)
				{
					return false;
				}
				++l_b; // next..!
				++r_b; // next..!
			}
			return l_b == l_e && r_b == r_e;
		}
	}

	friend auto operator==(const text<T>& lhs, const slice& rhs) -> bool
	{
		if (lhs.size() != rhs.tail - rhs.head)
		{
			return false;
		}
		const auto total {lhs.size() * sizeof(T)};
		// content equality without null terminator
		return std::memcmp(lhs.c_str(), rhs.head, total) == 0;
	}

	template<size_t N>
	friend auto operator==(const text<T>& lhs, const T (&rhs)[N]) -> bool
	{
		if (lhs.size() != N - 1)
		{
			return false;
		}
		// skip a null terminator, thus - 1
		const auto total {(N - 1) * sizeof(T)};
		// content equality without null terminator
		return std::memcmp(lhs.c_str(), rhs, total) == 0;
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator==(const T (&lhs)[N], const text<T>& rhs) -> bool
	{
		return rhs == lhs;
	}

	template<size_t N>
	// converting constructor
	friend auto operator==(const text<T>& lhs, const char8_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return lhs == text<char8_t>{rhs};
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator==(const char8_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return rhs == lhs;
	}

	template<size_t N>
	// converting constructor
	friend auto operator==(const text<T>& lhs, const char16_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return lhs == text<char16_t>{rhs};
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator==(const char16_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return rhs == lhs;
	}

	template<size_t N>
	// converting constructor
	friend auto operator==(const text<T>& lhs, const char32_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return lhs == text<char32_t>{rhs};
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator==(const char32_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return rhs == lhs;
	}

	//|------------|
	//| lhs != rhs |
	//|------------|

	template<typename U>
	friend auto operator!=(const text<T>& lhs, const text<U>& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	friend auto operator!=(const text<T>& lhs, const slice& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	friend auto operator!=(const text<T>& lhs, const T (&rhs)[N]) -> bool
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator!=(const T (&lhs)[N], const text<T>& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// converting constructor
	friend auto operator!=(const text<T>& lhs, const char8_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator!=(const char8_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// converting constructor
	friend auto operator!=(const text<T>& lhs, const char16_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator!=(const char16_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return !(rhs == lhs);
	}

	template<size_t N>
	// converting constructor
	friend auto operator!=(const text<T>& lhs, const char32_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend auto operator!=(const char32_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return !(rhs == lhs);
	}

	//|---------------------|
	//| traits::iterable<T> |
	//|---------------------|

	auto begin() const -> iterator
	{
		return {this->c_str()/* from RE:0 */};
	}

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

	auto is_lead(char16_t unit) -> bool
	{
		return 0xD800 <= unit && unit <= 0xDBFF;
	}

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
			assert(!"[ERROR]");
			std::unreachable();
		}
	}
}

template<>
void utf8::codec::decode(const char8_t* in, char32_t& out, width_t width)
{
	switch (width)
	{
		case +1:
		{
			out = in[0];
			break;
		}
		case +2:
		{
			out = 0;
			out |= (in[0] & 0x1F) << 06;
			out |= (in[1] & 0x3F) << 00;
			break;
		}
		case +3:
		{
			out = 0;
			out |= (in[0] & 0x0F) << 12;
			out |= (in[1] & 0x3F) << 06;
			out |= (in[2] & 0x3F) << 00;
			break;
		}
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
			assert(!"[ERROR]");
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
		case +1:
		{
			out[0] = in;
			break;
		}
		case +2:
		{
			const auto code {in - 0x10000};
			out[0] = 0xD800 | (code / 0x400);
			out[1] = 0xDC00 | (code & 0x3FF);
			break;
		}
		default:
		{
			assert(!"[ERROR]");
			std::unreachable();
		}
	}
}

template<>
void utf16::codec::decode(const char16_t* in, char32_t& out, width_t width)
{
	switch (width)
	{
		case +1:
		{
			out = in[0];
			break;
		}
		case +2:
		{
			out = 0x10000; // no plus op
			out |= (in[0] - 0xD800) << 10;
			out |= (in[1] - 0xDC00) << 00;
			break;
		}
		default:
		{
			assert(!"[ERROR]");
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
		// string_impl
		std::is_same_v<T, utf8>
		||
		std::is_same_v<T, utf16>
		||
		std::is_same_v<T, utf32>
		||
		// string_view
		std::is_same_v<T, utf8::slice>
		||
		std::is_same_v<T, utf16::slice>
		||
		std::is_same_v<T, utf32::slice>
	);

	template<typename T>
	concept string_impl =
	(
		std::is_same_v<T, utf8>
		||
		std::is_same_v<T, utf16>
		||
		std::is_same_v<T, utf32>
	);

	template<typename T>
	concept string_view =
	(
		std::is_same_v<T, utf8::slice>
		||
		std::is_same_v<T, utf16::slice>
		||
		std::is_same_v<T, utf32::slice>
	);
}
