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

inline /*Ი︵𐑼*/ auto operator<<(std::ostream& os, const char32_t code) -> std::ostream&
{
	char data[5];

	auto* ptr {data};

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

		inline constexpr auto begin() const -> const T* { return &this->data[0]; }
		inline constexpr auto begin()       ->       T* { return &this->data[0]; }

		inline constexpr auto end() const -> const T* { return &this->data[this->size]; }
		inline constexpr auto end()       ->       T* { return &this->data[this->size]; }

		inline constexpr auto operator[](const size_t nth) const -> const T& { return this->data[nth]; }
		inline constexpr auto operator[](const size_t nth)       ->       T& { return this->data[nth]; }
	};

	//|----------------------------<constants>----------------------------|
	static constexpr const uint8_t MAX {(sizeof(buffer) - 1) / (sizeof(T))};
	static constexpr const uint8_t RMB {(sizeof(buffer) - 1) * (    1    )};
	static constexpr const uint8_t SFT {IS_BIG ? (    1    ) : (    0    )};
	static constexpr const uint8_t MSK {IS_BIG ? 0b0000000'1 : 0b1'0000000};
	//|-------------------------------------------------------------------|

	static_assert(std::is_standard_layout_v<buffer>, "use other compiler");
	static_assert(std::is_trivially_copyable_v<buffer>, "use other compiler");

	static_assert(sizeof(buffer) == sizeof(size_t) * 3, "use other compiler");
	static_assert(alignof(buffer) == alignof(size_t) * 1, "use other compiler");

	static_assert(offsetof(buffer, data) == sizeof(size_t) * 0, "use other compiler");
	static_assert(offsetof(buffer, size) == sizeof(size_t) * 1, "use other compiler");

	#undef IS_BIG

	//|---------------------------------|
	//|              bytes              |
	//|---------------------------------|
	//|              small              |
	//|----------|----------|-----------|
	//|   data   |   size   |   extra   |
	//|----------|----------|-----------|

	#define chunk_t T
	union
	{
		//|-------------------------------|
		chunk_t small                      
		[sizeof(buffer) / sizeof(chunk_t)];
		//|-------------------------------|
		buffer large;
		//|-------------------------------|
		uint8_t bytes                      
		[sizeof(buffer) / sizeof(uint8_t)];
		//|-------------------------------|
	};
	#undef chunk_t

public:

	class slice;

private:

	template
	<
		typename S
	>
	requires
	(
		std::is_same_v<S, const slice&>
		||
		std::is_same_v<S,       slice&>
		||
		std::is_same_v<S, const text<T>&>
		||
		std::is_same_v<S,       text<T>&>
	)
	class proxy
	{
		S      src;
		size_t nth;

	public:

		proxy
		(
			decltype(src) src,
			decltype(nth) nth
		)
		: src(src), nth(nth) {}

		//|-----------------|
		//| member function |
		//|-----------------|

		// getter
		inline constexpr operator char32_t() const&& requires (std::is_same_v<std::remove_cvref_t<S>, text<T>>)
		{
			const T* ptr {this->src.c_str()};

			size_t i {0};
			size_t j {0};

			if constexpr (std::is_same_v<T, char32_t>)
			{
				if (this->nth < this->src.size())
				{
					i = this->nth;
					j = this->nth;
					goto for_utf32;
				}
				// no exception
				return U'\0';
			}

			for (; ptr[i]; ++j)
			{
				if (j == this->nth)
				{
					for_utf32:
					auto code {U'\0'};

					auto width {codec::next(&ptr[i])};
					codec::decode(&ptr[i], code, width);

					return code;
				}
				i += codec::next(&ptr[i]);
			}
			// no exception
			return U'\0';
		}

		// getter
		inline constexpr operator char32_t() const&& requires (std::is_same_v<std::remove_cvref_t<S>, slice>)
		{
			const T* ptr {this->src.head};

			size_t i {0};
			size_t j {0};

			if constexpr (std::is_same_v<T, char32_t>)
			{
				if (nth < this->src.size())
				{
					i = this->nth;
					j = this->nth;
					goto for_utf32;
				}
				// no exception
				return U'\0';
			}

			for (; ptr[i]; ++j)
			{
				if (j == nth)
				{
					for_utf32:
					auto code {U'\0'};

					auto width {codec::next(&ptr[i])};
					codec::decode(&ptr[i], code, width);

					return code;
				}
				i += codec::next(&ptr[i]);
			}
			// no exception
			return U'\0';
		}

		// setter
		inline constexpr auto operator=(const char32_t code)&& -> proxy& requires (!std::is_const_v<std::remove_reference<S>>)
		{
			const T* ptr {this->src.c_str()};

			size_t i {0};
			size_t j {0};

			if constexpr (std::is_same_v<T, char32_t>)
			{
				if (this->nth < this->src.size())
				{
					i = this->nth;
					j = this->nth;
					goto for_utf32;
				}
				// no exception
				return *this;
			}

			for (; ptr[i]; ++j)
			{
				if (j == this->nth)
				{
					for_utf32:
					const auto a {codec::next(&ptr[i])};
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
							const auto old_l {this->src.size()};
							const auto new_l {old_l + (b - a)};

							if (this->src.capacity() <= new_l)
							{
								this->src.capacity(new_l * 2);
							}
							// copy right => left
							std::ranges::copy_backward
							(
								&ptr[i + b],
								&ptr[old_l],
								&ptr[i + a]
							);
							// update metadata
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
							const auto old_l {this->src.size()};
							const auto new_l {old_l - (a - b)};

							// copy left => right
							std::ranges::copy
							(
								&ptr[i + b],
								&ptr[old_l],
								&ptr[i + a]
							);
							// update metadata
							this->src.size(new_l);
							break;
						}
					}
					//|---------<encoding>---------|
					codec::encode(code, &ptr[i], a);
					//|----------------------------|
				}
				i += this->src.width(ptr[i]);
			}
			// no exception
			return *this;
		}
	};

	class iterator
	{
		const T* ptr;

	public:

		iterator(decltype(ptr) ptr) : ptr(ptr) {}

		//|-----------------|
		//| member function |
		//|-----------------|

		inline constexpr auto operator&() const -> const T*
		{
			return this->ptr;
		}

		inline constexpr auto operator*() const -> char32_t
		{
			auto code {U'\0'};

			auto width {codec::next(this->ptr)};
			codec::decode(this->ptr, code, width);
			
			return code;
		}

		// prefix (++it)
		inline constexpr auto operator++() -> iterator&
		{
			// += is fine because [0 < rvalue]
			this->ptr += codec::next(this->ptr);

			return *this;
		}

		// postfix (it++)
		inline constexpr auto operator++(int) -> iterator
		{
			auto temp {*this};
			operator++();
			return temp;
		}

		// prefix (--it)
		inline constexpr auto operator--() -> iterator&
		{
			// += is fine because [rvalue < 0]
			this->ptr += codec::back(this->ptr);

			return *this;
		}

		// postfix (it--)
		inline constexpr auto operator--(int) -> iterator
		{
			auto temp {*this};
			operator--();
			return temp;
		} 
		//|------------|
		//| lhs == rhs |
		//|------------|

		friend constexpr auto operator==(const iterator& lhs, const iterator& rhs) -> bool
		{
			return lhs.ptr == rhs.ptr;
		}

		//|------------|
		//| lhs != rhs |
		//|------------|

		friend constexpr auto operator!=(const iterator& lhs, const iterator& rhs) -> bool
		{
			return lhs.ptr != rhs.ptr;
		}
	};

public:

	class codec
	{
		inline constexpr static const uint8_t TBL[]
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

		static constexpr auto is_lead(char16_t unit) -> bool
		{
			return 0xD800 <= unit && unit <= 0xDBFF;
		}

		static constexpr auto is_tail(char16_t unit) -> bool
		{
			return 0xDC00 <= unit && unit <= 0xDFFF;
		}

	public:

		// where : 0 < result
		static constexpr auto next(const T* ptr) -> int8_t
		{
			if constexpr (std::is_same_v<T, char8_t>)
			{
				return TBL[(*ptr >> 4) & 0x0F];
			}
			if constexpr (std::is_same_v<T, char16_t>)
			{
				if (is_lead(ptr[0])) return +2;
				if (is_tail(ptr[0])) return +1;
				                     return +1;
			}
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return +1;
			}
		}

		// where : result < 0
		static constexpr auto back(const T* ptr) -> int8_t
		{
			if constexpr (std::is_same_v<T, char8_t>)
			{
				int8_t i {-1};

				for (; (ptr[i] & 0xC0) == 0x80; --i);

				return i;
			}
			if constexpr (std::is_same_v<T, char16_t>)
			{
				if (is_tail(ptr[0])) return -2;
				if (is_lead(ptr[0])) return -1;
				                     return -1;
			}
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return -1;
			}
		}

		// where : 0 < result
		static constexpr auto width(const char32_t code) -> int8_t
		{
			if constexpr (std::is_same_v<T, char8_t>)
			{
				auto N {std::bit_width((uint32_t) code)};

				return 1  // 0xxxxxxx & 10xxxxxx * 0
				+
				( 8 <= N) // 110xxxxx & 10xxxxxx * 1
				+
				(12 <= N) // 1110xxxx & 10xxxxxx * 2
				+
				(17 <= N) // 11110xxx & 10xxxxxx * 3
				;
			}
			if constexpr (std::is_same_v<T, char16_t>)
			{
				return 1 + (0xFFFF < code);
			}
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return 1;
			}
		}

		static constexpr void encode(const char32_t in, T* out, int8_t width)
		{
			if constexpr (std::is_same_v<T, char8_t>)
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
						out[0] = 0xC0 | ((in >> 06) & 0x1F);
						out[1] = 0x80 | ((in >> 00) & 0x3F);
						break;
					}
					case +3:
					{
						out[0] = 0xE0 | ((in >> 12) & 0x0F);
						out[1] = 0x80 | ((in >> 06) & 0x3F);
						out[2] = 0x80 | ((in >> 00) & 0x3F);
						break;
					}
					case +4:
					{
						out[0] = 0xF0 | ((in >> 18) & 0x07);
						out[1] = 0x80 | ((in >> 12) & 0x3F);
						out[2] = 0x80 | ((in >> 06) & 0x3F);
						out[3] = 0x80 | ((in >> 00) & 0x3F);
						break;
					}
				}
			}
			if constexpr (std::is_same_v<T, char16_t>)
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
				}
			}
			if constexpr (std::is_same_v<T, char32_t>)
			{
				out[0] = in;
			}
		}

		static constexpr void decode(const T* in, char32_t& out, int8_t width)
		{
			if constexpr (std::is_same_v<T, char8_t>)
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
				}
			}
			if constexpr (std::is_same_v<T, char16_t>)
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
						out = 0x10000;
						out |= (in[0] - 0xD800) << 10;
						out |= (in[1] - 0xDC00) << 00;
						break;
					}
				}
			}
			if constexpr (std::is_same_v<T, char32_t>)
			{
				out = in[0];
			}
		}
	};

	class slice
	{
		friend text;

		const T* head;
		const T* tail;

	public:

		slice
		(
			decltype(head) head,
			decltype(tail) tail
		)
		: head {head}, tail {tail} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		inline constexpr auto size() const -> size_t
		{
			return this->tail - this->head;
		}

		inline constexpr auto empty() const -> bool
		{
			return this->size() == 0;
		}

		inline constexpr auto length() const -> size_t
		{
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return this->tail - this->head;
			}
			else // if (!std::is_same_v<T char32_t>)
			{
				// UTF-8/16
				const auto ptr {this->head};

				size_t i {0};
				size_t j {0};

				const auto max {this->size()};
				
				for (; i < max && ptr[i]; ++j)
				{
					//|-------<width>-------|
					i += codec::next(&ptr[i]);
					//|---------------------|
				}
				return j;
			}
		}
 
		//|--------------------------|
		//| utf8 <-> utf16 <-> utf32 |
		//|--------------------------|

		template<typename U>
		inline constexpr auto encode() const -> text<U>
		{
			text<U> str;
			// allocate
			str.capacity
			(
				[&]
				{
					size_t impl {0};

					for (const auto code : *this)
					{
						impl += text<U>::codec::width(code);
					}
					return impl + 1; // terminate
				}
				()
			);

			U* ptr {str.c_str()};

			for (const auto code : *this)
			{
				auto width {text<U>::codec::width(code)};
				text<U>::codec::encode(code, ptr, width);
				ptr += width; // move to next code point
			}
			str.size(ptr - str.c_str());

			return str;
		}

		inline constexpr auto to_utf8() const -> text<char8_t>
		{
			return this->encode<char8_t>();
		}

		inline constexpr auto to_utf16() const -> text<char16_t>
		{
			return this->encode<char16_t>();
		}

		inline constexpr auto to_utf32() const -> text<char32_t>
		{
			return this->encode<char32_t>();
		}

		//|--------------|
		//| lhs.substr() |
		//|--------------|

		inline constexpr auto substr(const size_t start, size_t count = SIZE_MAX) const -> slice
		{
			const T* ptr {this->head};

			count = std::min(count, this->size());
			
			if constexpr (std::is_same_v<T, char32_t>)
			{
				return {&ptr[start], &ptr[start + count]};
			}
			else // if (!std::is_same_v<T, char32_t>)
			{
				size_t a {0};

				for (size_t i {0}; ptr[a] && i < start; ++i)
				{
					a += codec::next(&ptr[a]);
				}
				// continue
				size_t b {a};

				for (size_t i {0}; ptr[b] && i < count; ++i)
				{
					b += codec::next(&ptr[b]);
				}
				return {&ptr[a], &ptr[b]};
			}
		}

		//|------------|
		//| lhs.find() |
		//|------------|

		inline constexpr auto find(const text<T>& str, const size_t offset = 0) const -> size_t
		{
			const T* ptr {this->head};

			size_t nth {0};
			
			// skip offset
			for (; nth < offset; ++nth)
			{
				ptr += codec::next(ptr);
			}

			auto out {U'\0'};

			// whilst remaining
			while (ptr < this->tail)
			{
				auto width {codec::next(ptr)};
				codec::decode(ptr, out, width);

				// 1st char match
				if (out == str[0])
				{
					const T* temp {ptr};

					size_t i {0};

					// try match
					for (const auto code : str)
					{
						if (this->tail <= temp)
						{
							break;
						}
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
		inline constexpr auto find(const T (&str)[N], const size_t offset = 0) const -> size_t
		{
			const T* ptr {this->head};

			size_t nth {0};

			// skip offset
			for (; nth < offset; ++nth)
			{
				ptr += codec::next(ptr);
			}

			auto out {U'\0'};

			// whilst remaining
			while (ptr < this->tail)
			{
				auto width {codec::next(ptr)};
				codec::decode(ptr, out, width);

				// 1st char match
				if (out == str[0])
				{
					const T* temp {ptr};

					size_t i {0};

					// try match
					for (const auto code : str)
					{
						if (this->tail <= temp)
						{
							break;
						}
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

		inline constexpr auto find(const char32_t code, const size_t offset = 0) const -> size_t
		{
			const T* ptr {this->head};

			size_t nth {0};

			// skip offset
			for (; nth < offset; ++nth)
			{
				ptr += codec::next(ptr);
			}

			auto out {U'\0'};

			// whilst remaining
			while (ptr < this->tail)
			{
				auto width {codec::next(ptr)};
				codec::decode(ptr, out, width);

				// 1st char match
				if (out == code)
				{
					return nth - offset;
				}
				ptr += width; ++nth;
			}
			return SIZE_MAX;
		}

		template<typename U>
		inline constexpr auto find(const text<U>& str, const size_t offset = 0) const -> auto
		{
			return this->find(str.template encode<T>(), offset);
		}

		template<size_t N>
		// converting constructor
		inline constexpr auto find(const char8_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char8_t>)
		{
			return this->find(text<char8_t> {str}, offset);
		}

		template<size_t N>
		// converting constructor
		inline constexpr auto find(const char16_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char16_t>)
		{
			return this->find(text<char16_t> {str}, offset);
		}

		template<size_t N>
		// converting constructor
		inline constexpr auto find(const char32_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char32_t>)
		{
			return this->find(text<char32_t> {str}, offset);
		}

		//|-------------|
		//| lhs.split() |
		//|-------------|

		inline constexpr auto split(const text<T>& str) const -> std::vector<slice>
		{
			std::vector<slice> result;

			const T* head {this->head};
			const T* tail {this->head};

			const auto N {str.size()};

			while (tail < this->tail)
			{
				if (head + N - 1 < this->tail)
				{
					size_t i {0};

					// try match
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
						// skip over matched part
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

		template<size_t N>
		inline constexpr auto split(const T (&str)[N]) const -> std::vector<slice>
		{
			std::vector<slice> result;

			const T* head {this->head};
			const T* tail {this->head};

			while (tail < this->tail)
			{
				if (head + N - 1 < this->tail)
				{
					size_t i {0};

					// try match
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
						// skip over matched part
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

		inline constexpr auto split(const char32_t code) const -> std::vector<slice>
		{
			std::vector<slice> result;

			const T* head {this->head};
			const T* tail {this->head};

			auto out {U'\0'};

			while (tail < this->tail)
			{
				auto width {codec::next(tail)};
				codec::decode(tail, out, width);

				if (out == code)
				{
					result.emplace_back(head, tail);
					// skip over matched part
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

		template<typename U>
		inline constexpr auto split(const text<U>& str) const -> std::vector<slice>
		{
			std::vector<slice> result;
			
			const T* head {this->head};
			const T* tail {this->head};

			auto out {U'\0'};

			while (tail < this->tail)
			{
				auto width {codec::next(tail)};
				codec::decode(tail, out, width);

				// 1st char match
				if (out == str[0])
				{
					const T* ptr {tail};

					size_t i {0};

					// try match
					for (const auto code : str)
					{
						if (this->tail <= ptr)
						{
							break;
						}
						auto width {codec::next(ptr)};
						codec::decode(ptr, out, width);

						if (out != code)
						{
							break;
						}
						// increase both ptr & i
						ptr += width; ++i;
					}
					// if full match
					if (i == str.length())
					{
						result.emplace_back(head, tail);
						// skip over matched part
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

		template<size_t N>
		// converting constructor
		inline constexpr auto split(const char8_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char8_t>)
		{
			return this->split(text<char8_t> {str});
		}

		template<size_t N>
		// converting constructor
		inline constexpr auto split(const char16_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char16_t>)
		{
			return this->split(text<char16_t> {str});
		}

		template<size_t N>
		// converting constructor
		inline constexpr auto split(const char32_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char32_t>)
		{
			return this->split(text<char32_t> {str});
		}

		//|-----------|
		//| lhs < rhs |
		//|-----------|

		friend constexpr auto operator<(const slice& lhs, const slice& rhs) -> bool
		{
			const auto len {std::min
			(lhs.size(), rhs.size())};

			const T* foo {lhs.head};
			const T* bar {rhs.head};

			for (size_t i {0}; i < len; ++i)
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

		inline constexpr auto operator[](const size_t nth) const -> proxy<decltype(*this)>
		{
			return {*this, nth};
		}

		inline constexpr auto operator[](const size_t nth)       -> proxy<decltype(*this)>
		{
			return {*this, nth};
		}

		//|------------|
		//| lhs == rhs |
		//|------------|

		friend constexpr auto operator==(const slice& lhs, const text<T>& rhs) -> bool
		{
			return lhs.size() == rhs.size() && !std::memcmp
			(
				lhs.head, rhs.c_str(), rhs.size() * sizeof(T)
			);
		}

		friend constexpr auto operator==(const slice& lhs, const slice& rhs) -> bool
		{
			return
			(
				lhs.head == rhs.head
				&&
				lhs.tail == rhs.tail
			);
		}

		template<size_t N>
		friend constexpr auto operator==(const slice& lhs, const T (&rhs)[N]) -> bool
		{
			return lhs.size() == N - 1 && !std::memcmp
			(
				lhs.head, rhs, (N - 1) * sizeof(T)
			);
		}

		template<typename U>
		friend constexpr auto operator==(const slice& lhs, const text<U>& rhs) -> bool
		{
			auto l_b {lhs.begin()}; auto l_e {lhs.end()};
			auto r_b {rhs.begin()}; auto r_e {rhs.end()};

			for (;l_b != l_e || r_b != r_e; ++l_b, ++r_b)
			{
				if (*l_b != *r_b)
				{
					return false;
				}
			}
			return l_b == l_e && r_b == r_e;
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator==(const T (&lhs)[N], const slice& rhs) -> bool
		{
			return rhs == lhs;
		}

		template<size_t N>
		// converting constructor
		friend constexpr auto operator==(const slice& lhs, const char8_t (&rhs)[N]) -> bool requires (!std::is_same_v<T, char8_t>)
		{
			return lhs == text<char8_t> {rhs};
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator==(const char8_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char8_t>)
		{
			return rhs == lhs;
		}

		template<size_t N>
		// converting constructor
		friend constexpr auto operator==(const slice& lhs, const char16_t (&rhs)[N]) -> bool requires (!std::is_same_v<T, char16_t>)
		{
			return lhs == text<char16_t> {rhs};
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator==(const char16_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char16_t>)
		{
			return rhs == lhs;
		}

		template<size_t N>
		// converting constructor
		friend constexpr auto operator==(const slice& lhs, const char32_t (&rhs)[N]) -> bool requires (!std::is_same_v<T, char32_t>)
		{
			return lhs == text<char32_t> {rhs};
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator==(const char32_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char32_t>)
		{
			return rhs == lhs;
		}

		//|------------|
		//| lhs != rhs |
		//|------------|

		friend constexpr auto operator!=(const slice& lhs, const text<T>& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		friend constexpr auto operator!=(const slice& lhs, const slice& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		friend constexpr auto operator!=(const slice& lhs, const T (&rhs)[N]) -> bool
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		friend constexpr auto operator!=(const T (&lhs)[N], const slice& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		template<typename U>
		friend constexpr auto operator!=(const slice& lhs, const text<U>& rhs) -> bool
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// converting constructor
		friend constexpr auto operator!=(const slice& lhs, const char8_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char8_t>)
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator!=(const char8_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char8_t>)
		{
			return rhs != lhs;
		}

		template<size_t N>
		// converting constructor
		friend constexpr auto operator!=(const slice& lhs, const char16_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char16_t>)
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator!=(const char16_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char16_t>)
		{
			return rhs != lhs;
		}

		template<size_t N>
		// converting constructor
		friend constexpr auto operator!=(const slice& lhs, const char32_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char32_t>)
		{
			return !(lhs == rhs);
		}

		template<size_t N>
		// reverse op overloading
		friend constexpr auto operator!=(const char32_t (&lhs)[N], const slice& rhs) -> bool requires (!std::is_same_v<T, char32_t>)
		{
			return rhs != lhs;
		}

		//|--------------------|
		//| trait::iterable<T> |
		//|--------------------|

		inline constexpr auto begin() const -> iterator { return {this->head}; }
		inline constexpr auto begin()       -> iterator { return {this->head}; }

		inline constexpr auto end() const -> iterator { return {this->tail}; }
		inline constexpr auto end()       -> iterator { return {this->tail}; }

		//|---------------------|
		//| trait::printable<T> |
		//|---------------------|

		friend constexpr auto operator<<(std::ostream& os, const slice& str) -> std::ostream&
		{
			for (const auto code : str)
			{
				os << code;
			}
			return os; // for chaining
		}
	};

	class format
	{
		// fragments of source
		std::vector<text<T>> atom;
		// arguments to concat
		std::vector<text<T>> frag;

		inline constexpr auto full() const -> bool
		{
			return // until N = N' + 1
			(
				this->atom.size() + 0
				==
				this->frag.size() + 1
			);
		}

	public:

		format(const slice& str)
		{
			if constexpr (std::same_as<T, char8_t>)
			{
				for (auto& _ : str.split(u8"%s"))
				{
					this->atom.emplace_back(_);
				}
			}
			if constexpr (std::same_as<T, char16_t>)
			{
				for (auto& _ : str.split(u"%s"))
				{
					this->atom.emplace_back(_);
				}
			}
			if constexpr (std::same_as<T, char32_t>)
			{
				for (auto& _ : str.split(U"%s"))
				{
					this->atom.emplace_back(_);
				}
			}
		}

		format(const text<T>& str)
		{
			if constexpr (std::same_as<T, char8_t>)
			{
				for (auto& _ : str.split(u8"%s"))
				{
					this->atom.emplace_back(_);
				}
			}
			if constexpr (std::same_as<T, char16_t>)
			{
				for (auto& _ : str.split(u"%s"))
				{
					this->atom.emplace_back(_);
				}
			}
			if constexpr (std::same_as<T, char32_t>)
			{
				for (auto& _ : str.split(U"%s"))
				{
					this->atom.emplace_back(_);
				}
			}
		}

		//|-----------------|
		//| member function |
		//|-----------------|

		operator text<T>()
		{
			while (!this->full())
			{
				if constexpr (std::same_as<T, char8_t>)
				{
					this->frag.emplace_back(u8"%s");
				}
				if constexpr (std::same_as<T, char16_t>)
				{
					this->frag.emplace_back(u"%s");
				}
				if constexpr (std::same_as<T, char32_t>)
				{
					this->frag.emplace_back(U"%s");
				}
			}

			text<T> str;
			// allocate
			str.capacity
			(
				[&]
				{
					size_t impl {0};

					for (const auto& _ : this->atom)
					{
						impl += _.size();
					}
					for (const auto& _ : this->frag)
					{
						impl += _.size();
					}
					return impl + 1;
				}
				()
			);

			// mix and join
			for (size_t i {0}; i < frag.size(); ++i)
			{
				str += this->atom[i]; // concat atom
				str += this->frag[i]; // concat frag
			}
			// concat last atom
			str += this->atom.back();

			return str;
		}

		//|-----------|
		//| lhs | rhs |
		//|-----------|

		inline constexpr auto operator|(const text<T>& rhs) -> format&
		{
			if (!this->full())
			{
				this->frag.emplace_back(rhs);
			}
			return *this;
		}

		inline constexpr auto operator|(const slice& rhs) -> format&
		{
			if (!this->full())
			{
				this->frag.emplace_back(rhs);
			}
			return *this;
		}

		template<size_t N>
		inline constexpr auto operator|(const T (&rhs)[N]) -> format&
		{
			if (!this->full())
			{
				this->frag.emplace_back(rhs);
			}
			return *this;
		}
	
		template<typename I> requires (std::is_integral_v<I>)
		inline constexpr auto operator|(const I rhs) -> format&
		{
			if (!this->full())
			{
				this->frag.emplace_back(reinterpret_cast
				<const T*>(std::to_string(rhs).c_str()));
			}
			return *this;
		}

		template<typename I> requires (std::is_floating_point_v<I>)
		inline constexpr auto operator|(const I rhs) -> format&
		{
			if (!this->full())
			{
				this->frag.emplace_back(reinterpret_cast
				<const T*>(std::to_string(rhs).c_str()));
			}
			return *this;
		}
	};

	// a -> b
	COPY_CALL(text<T>)
	{
		const auto N {from.size()};

		if (dest.capacity() <= N)
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

	constexpr text() : bytes {0}
	{
		this->bytes[RMB] = MAX << SFT;
		// then...
		assert(this->mode() == tag::SMALL);
	}

	template<size_t N>
	requires (N <= MAX)
	constexpr text(const T (&str)[N]) : text()
	{
		this->size(N);
		// check mode
		assert(this->mode() == tag::SMALL);
		// write data
		std::ranges::copy(str, str + N, this->small);
	}

	template<size_t N>
	requires (MAX < N)
	constexpr text(const T (&str)[N]) : text()
	{
		this->size(N);
		// check mode
		assert(this->mode() == tag::LARGE);
		// write data
		std::ranges::copy(str, str + N, this->large.data);
	}

	constexpr text(const T* str) : text()
	{
		if (str != nullptr)
		{
			const auto N {std::char_traits<T>::length(str)};
			// copy meta
			this->size(N);
			// copy data
			std::ranges::copy(str, str + N + 1, this->c_str());
		}
	}

	template<typename S>
	requires requires (S str)
	{
		str.to_utf8(); // interface
		str.to_utf16(); // interface
		str.to_utf32(); // interface

		!std::is_same_v<S, text<char8_t>>;
		!std::is_same_v<S, text<char16_t>>;
		!std::is_same_v<S, text<char32_t>>;
	}
	constexpr text(const S& str) : text()
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

	template<typename S>
	requires requires (S str)
	{
		str.to_utf8(); // interface
		str.to_utf16(); // interface
		str.to_utf32(); // interface

		!std::is_same_v<S, text<char8_t>>;
		!std::is_same_v<S, text<char16_t>>;
		!std::is_same_v<S, text<char32_t>>;
	}
	inline constexpr auto operator=(const S& rhs) -> text&
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

	//|------------------|
	//| member functions |
	//|------------------|

	// for memory access
	inline constexpr auto c_str() const -> const T*
	{
		switch (this->mode())
		{
			case tag::SMALL: { return this->small     ; }
			case tag::LARGE: { return this->large.data; }
		}
		assert(false && "-Wreturn-type");
	}

	// for memory access
	inline constexpr auto c_str()       ->       T*
	{
		switch (this->mode())
		{
			case tag::SMALL: { return this->small     ; }
			case tag::LARGE: { return this->large.data; }
		}
		assert(false && "-Wreturn-type");
	}

	[[deprecated("legacy API")]]
	inline constexpr operator const char*() const requires (std::is_same_v<T, char8_t>)
	{
		return reinterpret_cast<const char*>(this->c_str());
	}

	[[deprecated("legacy API")]]
	inline constexpr operator       char*()       requires (std::is_same_v<T, char8_t>)
	{
		return reinterpret_cast<      char*>(this->c_str());
	}

	//|-------------------------------------------------------|-------|
	//|                                                       |   ↓   |
	//|---------------|-------|-------|-------|-------|-------|-------|
	//| 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit |
	//|-------|-------|-------|-------|-------|-------|-------|-------|

	// getter
	inline constexpr auto mode() const -> tag
	{
		return static_cast<tag>(this->bytes[RMB] & MSK);
	}

	//|-------------------------------------------------------|-------|
	//|   ↓       ↓       ↓       ↓       ↓       ↓       ↓   |       |
	//|---------------|-------|-------|-------|-------|-------|-------|
	//| 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit | 1 bit |
	//|-------|-------|-------|-------|-------|-------|-------|-------|

	// getter
	inline constexpr auto size() const -> size_t
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
	inline constexpr auto size(const size_t value)
	{
		if (this->capacity() <= value)
		{
			allocate:
			auto data {new T[value + 1]};

			switch (this->mode())
			{
				// S -> L
				case tag::SMALL:
				{
					std::ranges::copy(this->small, data); /* cannot delete stack */; break;
				}
				// L -> L
				case tag::LARGE:
				{
					std::ranges::copy(this->large, data); delete[] this->large.data; break;
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

					const auto slot {MAX - value};

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

	inline constexpr auto empty() const -> bool
	{
		return this->size() == 0;
	}

	inline constexpr auto length() const -> size_t
	{
		// UTF-32
		if constexpr (std::is_same_v<T, char32_t>)
		{
			return this->size();
		}
		// UTF-8/16
		const T* ptr {this->c_str()};

		size_t i {0};
		size_t j {0};

		for (; ptr[i]; ++j)
		{
			i += codec::next(ptr + i);
		}
		return j;
	}

	// getter
	inline constexpr auto capacity() const -> size_t
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
	inline constexpr auto capacity(const size_t value)
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
					std::ranges::copy(this->small, data); /* cannot delete stack */; break;
				}
				// L -> L
				case tag::LARGE:
				{
					std::ranges::copy(this->large, data); delete[] this->large.data; break;
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

	//|--------------------------|
	//| utf8 <-> utf16 <-> utf32 |
	//|--------------------------|

	template<typename U>
	inline constexpr auto encode() const -> text<U>
	{
		if constexpr (std::is_same_v<T, U>)
		{
			return *this;
		}
		else // if (!std::is_same_v<T, U>)
		{
			text<U> str;
			// allocate
			str.capacity
			(
				[&]
				{
					size_t impl {0};

					for (const auto code : *this)
					{
						impl += text<U>::codec::width(code);
					}
					return impl + 1; // terminate
				}
				()
			);
			
			U* ptr {str.c_str()};

			for (const auto code : *this)
			{
				auto width {text<U>::codec::width(code)};
				text<U>::codec::encode(code, ptr, width);
				ptr += width; // move to next code point
			}
			str.size(ptr - str.c_str());

			return str;
		}
	}

	inline constexpr auto to_utf8() const -> text<char8_t>
	{
		return this->encode<char8_t>();
	}

	inline constexpr auto to_utf16() const -> text<char16_t>
	{
		return this->encode<char16_t>();
	}

	inline constexpr auto to_utf32() const -> text<char32_t>
	{
		return this->encode<char32_t>();
	}

	//|--------------|
	//| lhs.substr() |
	//|--------------|

	inline constexpr auto substr(const size_t start, size_t count = SIZE_MAX) const -> slice
	{
		const T* ptr {this->c_str()};

		count = std::min(count, this->size());
	
		if constexpr (std::is_same_v<T, char32_t>)
		{
			return {&ptr[start], &ptr[start + count]};
		}
		else // if (!std::is_same_v<T, char32_t>)
		{
			size_t a {0};

			for (size_t i {0}; ptr[a] && i < start; ++i)
			{
				a += codec::next(&ptr[a]);
			}
			// continue
			size_t b {a};

			for (size_t i {0}; ptr[b] && i < count; ++i)
			{
				b += codec::next(&ptr[b]);
			}
			return {&ptr[a], &ptr[b]};
		}
	}

	//|------------|
	//| lhs.find() |
	//|------------|

	inline constexpr auto find(const text<T>& str, const size_t offset = 0) const -> size_t
	{
		const T* END {&this->c_str()[this->size()]};
		const T* ptr {&this->c_str()[0x0000000000]};

		size_t nth {0};
		
		// skip offset
		for (; nth < offset; ++nth)
		{
			ptr += codec::next(ptr);
		}
		
		auto out {U'\0'};

		// whilst remaining
		while (ptr < END)
		{
			auto width {codec::next(ptr)};
			codec::decode(ptr, out, width);

			// 1st char match
			if (out == str[0])
			{
				const T* temp {ptr};

				size_t i {0};

				// try match
				for (const auto code : str)
				{
					if (END <= temp)
					{
						break;
					}
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
	inline constexpr auto find(const T (&str)[N], const size_t offset = 0) const -> size_t
	{
		const T* END {&this->c_str()[this->size()]};
		const T* ptr {&this->c_str()[0x0000000000]};

		size_t nth {0};

		// skip offset
		for (; nth < offset; ++nth)
		{
			ptr += codec::next(ptr);
		}

		auto out {U'\0'};

		// whilst remaining
		while (ptr < END)
		{
			auto width {codec::next(ptr)};
			codec::decode(ptr, out, width);

			// 1st char match
			if (out == str[0])
			{
				const T* temp {ptr};

				size_t i {0};

				// try match
				for (const auto code : str)
				{
					if (END <= temp)
					{
						break;
					}
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

	inline constexpr auto find(const char32_t code, const size_t offset = 0) const -> size_t
	{
		const T* END {&this->c_str()[this->size()]};
		const T* ptr {&this->c_str()[0x0000000000]};

		size_t nth {0};

		// skip offset
		for (; nth < offset; ++nth)
		{
			ptr += codec::next(ptr);
		}

		auto out {U'\0'};

		// whilst remaining
		while (ptr < END)
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

	template<typename U>
	inline constexpr auto find(const text<U>& str, const size_t offset = 0) const -> size_t
	{
		return this->find(str.template encode<T>(), offset);
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto find(const char8_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return this->find(text<char8_t> {str}, offset);
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto find(const char16_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return this->find(text<char16_t> {str}, offset);
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto find(const char32_t (&str)[N], const size_t offset = 0) const -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return this->find(text<char32_t> {str}, offset);
	}

	//|-------------|
	//| lhs.split() |
	//|-------------|

	inline constexpr auto split(const text<T>& str) const -> std::vector<slice>
	{
		const T* END {&this->c_str()[this->size()]};

		std::vector<slice> result;
		
		const T* head {this->c_str()};
		const T* tail {this->c_str()};

		const auto N {str.size()};

		while (tail < END)
		{
			if (head + N - 1 < END)
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
					// skip over matched part
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
		if (head <= END)
		{
			result.emplace_back(head, END);
		}
		return result;
	}

	template<size_t N>
	inline constexpr auto split(const T (&str)[N]) const -> std::vector<slice>
	{
		const T* END {&this->c_str()[this->size()]};

		std::vector<slice> result;

		const T* head {this->c_str()};
		const T* tail {this->c_str()};

		while (tail < END)
		{
			if (head + N - 1 < END)
			{
				size_t i {0};

				// try match
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
					// skip over matched part
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
		if (head <= END)
		{
			result.emplace_back(head, END);
		}
		return result;
	}

	inline constexpr auto split(const char32_t code) const -> std::vector<slice>
	{
		const T* END {&this->c_str()[this->size()]};
	
		std::vector<slice> result;
		
		const T* head {this->c_str()};
		const T* tail {this->c_str()};

		auto out {U'\0'};

		while (tail < END)
		{
			auto width {codec::next(tail)};
			codec::decode(tail, out, width);

			if (out == code)
			{
				result.emplace_back(head, tail);
				// skip over matched part
				head = tail + width;
			}
			// move point to the next
			tail += width;
		}
		// rest of the slice
		if (head <= END)
		{
			result.emplace_back(head, END);
		}
		return result;
	}

	template<typename U>
	inline constexpr auto split(const text<U>& str) const -> std::vector<slice>
	{
		const T* END {&this->c_str()[this->size()]};

		std::vector<slice> result;
		
		const T* head {this->c_str()};
		const T* tail {this->c_str()};

		auto out {U'\0'};
		
		while (tail < END)
		{
			auto width {codec::next(tail)};
			codec::decode(tail, out, width);

			if (out == str[0])
			{
				const T* ptr {tail};

				size_t i {0};

				// try match
				for (const auto code : str)
				{
					if (END <= ptr)
					{
						break;
					}
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
					// skip over matched part
					head = tail = ptr;
					// avoid goto?
					continue;
				}
			}
			tail += width;
		}
		// rest of the slice
		if (head <= END)
		{
			result.emplace_back(head, END);
		}
		return result;
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto split(const char8_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return this->split(text<char8_t> {str});
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto split(const char16_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return this->split(text<char16_t> {str});
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto split(const char32_t (&str)[N]) const -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return this->split(text<char32_t> {str});
	}

	//|--------|
	//| lhs[N] |
	//|--------|

	inline constexpr auto operator[](const size_t nth) const -> proxy<decltype(*this)>
	{
		return {*this, nth};
	}

	inline constexpr auto operator[](const size_t nth)       -> proxy<decltype(*this)>
	{
		return {*this, nth};
	}

	//|-----------|
	//| lhs < rhs |
	//|-----------|

	friend constexpr auto operator<(const text<T>& lhs, const text<T>& rhs) -> bool
	{
		const auto len {std::min
		(lhs.size(), rhs.size())};

		const auto foo {lhs.c_str()};
		const auto bar {rhs.c_str()};

		for (size_t i {0}; i < len; ++i)
		{
			if (foo[i] != bar[i])
			{
				return foo[i] < bar[i];
			}
		}
		return lhs.size() < rhs.size();
	}

	//|------------|
	//| lhs += rhs |
	//|------------|

	inline constexpr auto operator+=(const text<T>& rhs) -> text<T>
	{
		//|------------<content size>------------|
		const auto MAX {this->size() + rhs.size()};
		//|--------------------------------------|

		if (this->capacity() <= MAX)
		{
			this->capacity(MAX + 1);
		}
		auto const N {rhs.size()};

		std::ranges::copy(&rhs.c_str()[0], &rhs.c_str()[N],
		// destination
		&this->c_str()[this->size()]);

		// set metadata
		this->size(MAX);

		return *this;
	}

	inline constexpr auto operator+=(const slice& rhs) -> text<T>
	{
		const auto MAX {this->size() + rhs.size()};

		if (this->capacity() <= MAX)
		{
			this->capacity(MAX + 1);
		}

		std::ranges::copy(&rhs.head[0], &rhs.tail[0],
		// destination
		&this->c_str()[this->size()]);
		
		// set metadata
		this->size(MAX);

		return *this;
	}

	template<size_t N>
	inline constexpr auto operator+=(const T (&rhs)[N]) -> text<T>
	{
		const auto MAX {this->size() + N - 1};

		if (this->capacity() <= MAX)
		{
			this->capacity(MAX + 1);
		}

		std::ranges::copy(&rhs[0], &rhs[N],
		// destination
		&this->c_str()[this->size()]);

		// set metadata
		this->size(MAX);

		return *this;
	}

	template<typename U>
	inline constexpr auto operator+=(const text<U>& rhs) -> text<T>
	{
		return *this += rhs.template encode<T>();
	}

	//|-----------|
	//| lhs + rhs |
	//|-----------|

	template<typename U>
	friend constexpr auto operator+(const text<T>& lhs, const text<U>& rhs) -> text<T>
	{
		text<T> str {lhs.size() + rhs.size()};

		// copy lhs
		str += lhs;
		// copy rhs
		str += rhs;

		return str;
	}

	friend constexpr auto operator+(const text<T>& lhs, const slice& rhs) -> text<T>
	{
		text<T> str {lhs.size() + rhs.size()};

		// copy lhs
		str += lhs;
		// copy rhs
		str += rhs;

		return str;
	}

	// reverse op overloading
	friend constexpr auto operator+(const slice& lhs, const text<T> rhs) -> text<T>
	{
		text<T> str {lhs.size() + rhs.size()};

		// copy lhs
		str += lhs;
		// copy rhs
		str += rhs;

		return str;
	}

	template<size_t N>
	friend constexpr auto operator+(const text<T>& lhs, const T (&rhs)[N]) -> text<T>
	{
		text<T> str {lhs.size() + N - 1};

		// copy lhs
		str += lhs;
		// copy rhs
		str += rhs;

		return str;
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator+(const T (&lhs)[N], const text<T> rhs) -> text<T>
	{
		text<T> str {N - 1 + rhs.size()};

		// copy lhs
		str += lhs;
		// copy rhs
		str += rhs;

		return str;
	}

	//|------------|
	//| lhs == rhs |
	//|------------|

	friend constexpr auto operator==(const text<T>& lhs, const text<T>& rhs) -> bool
	{
		return lhs.size() == rhs.size() && !std::memcmp
		(
			lhs.c_str(), rhs.c_str(), lhs.size() * sizeof(T)
		);
	}

	friend constexpr auto operator==(const text<T>& lhs, const slice& rhs) -> bool
	{
		return lhs.size() == rhs.size() && !std::memcmp
		(
			lhs.c_str(), rhs.head, lhs.size() * sizeof(T)
		);
	}

	template<size_t N>
	friend constexpr auto operator==(const text<T>& lhs, const T (&rhs)[N]) -> bool
	{
		return lhs.size() == N - 1 && !std::memcmp
		(
			lhs.c_str(), rhs, (N - 1) * sizeof(T)
		);
	}

	template<typename U>
	friend constexpr auto operator==(const text<T>& lhs, const text<U>& rhs) -> bool
	{
		auto l_b {lhs.begin()}; auto l_e {lhs.end()};
		auto r_b {rhs.begin()}; auto r_e {rhs.end()};

		for (;l_b != l_e || r_b != r_e; ++l_b, ++r_b)
		{
			if (*l_b != *r_b)
			{
				return false;
			}
		}
		return l_b == l_e && r_b == r_e;
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator==(const T (&lhs)[N], const text<T>& rhs) -> bool
	{
		return rhs == lhs;
	}

	template<size_t N>
	// converting constructor
	friend constexpr auto operator==(const text<T>& lhs, const char8_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return lhs == text<char8_t> {rhs};
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator==(const char8_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return rhs == lhs;
	}

	template<size_t N>
	// converting constructor
	friend constexpr auto operator==(const text<T>& lhs, const char16_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return lhs == text<char16_t> {rhs};
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator==(const char16_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return rhs == lhs;
	}

	template<size_t N>
	// converting constructor
	friend constexpr auto operator==(const text<T>& lhs, const char32_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return lhs == text<char32_t> {rhs};
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator==(const char32_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return rhs == lhs;
	}

	//|------------|
	//| lhs != rhs |
	//|------------|

	friend constexpr auto operator!=(const text<T>& lhs, const text<T>& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	friend constexpr auto operator!=(const text<T>& lhs, const slice& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	friend constexpr auto operator!=(const text<T>& lhs, const T (&rhs)[N]) -> bool
	{
		return !(lhs == rhs);
	}

	template<typename U>
	friend constexpr auto operator!=(const text<T>& lhs, const text<U>& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator!=(const T (&lhs)[N], const text<T>& rhs) -> bool
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// converting constructor
	friend constexpr auto operator!=(const text<T>& lhs, const char8_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator!=(const char8_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char8_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// converting constructor
	friend constexpr auto operator!=(const text<T>& lhs, const char16_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator!=(const char16_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char16_t>)
	{
		return !(rhs == lhs);
	}

	template<size_t N>
	// converting constructor
	friend constexpr auto operator!=(const text<T>& lhs, const char32_t (&rhs)[N]) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return !(lhs == rhs);
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator!=(const char32_t (&lhs)[N], const text<T>& rhs) -> auto requires (!std::is_same_v<T, char32_t>)
	{
		return !(rhs == lhs);
	}

	//|-----------|
	//| lhs | rhs |
	//|-----------|

	friend constexpr auto operator|(const text<T>& lhs, const text<T>& rhs) -> format
	{
		return format {lhs} | rhs;
	}

	friend constexpr auto operator|(const text<T>& lhs, const slice& rhs) -> format
	{
		return format {lhs} | rhs;
	}

	friend constexpr auto operator|(const slice& lhs, const text<T>& rhs) -> format
	{
		return format {lhs} | rhs;
	}

	template<size_t N>
	friend constexpr auto operator|(const text<T>& lhs, const T (&rhs)[N]) -> format
	{
		return format {lhs} | rhs;
	}

	template<size_t N>
	friend constexpr auto operator|(const T (&lhs)[N], const text<T>& rhs) -> format
	{
		return format {lhs} | rhs;
	}

	template<typename U>
	friend constexpr auto operator|(const text<T>& lhs, const text<U>& rhs) -> format
	{
		return format {lhs} | rhs;
	}

	template<typename I> requires (std::is_integral_v<I>)
	friend constexpr auto operator|(const text<T>& lhs, const I rhs) -> format
	{
		return format {lhs} | reinterpret_cast<const T*>(std::to_string(rhs).c_str());
	}

	template<typename I> requires (std::is_floating_point_v<I>)
	friend constexpr auto operator|(const text<T>& lhs, const I rhs) -> format
	{
		return format {lhs} | reinterpret_cast<const T*>(std::to_string(rhs).c_str());
	}

	template<typename I> requires (std::is_integral_v<I>)
	friend constexpr auto operator|(const slice& lhs, const I rhs) -> format
	{
		return format {lhs} | reinterpret_cast<const T*>(std::to_string(rhs).c_str());
	}

	template<typename I> requires (std::is_floating_point_v<I>)
	friend constexpr auto operator|(const slice& lhs, const I rhs) -> format
	{
		return format {lhs} | reinterpret_cast<const T*>(std::to_string(rhs).c_str());
	}

	template<size_t N>
	friend constexpr auto operator|(const text<T>& lhs, const char8_t (&rhs)[N]) -> format requires (!std::is_same_v<T, char8_t>)
	{
		return format {lhs} | text<char8_t> {rhs}.encode<T>();
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator|(const char8_t (&lhs)[N], const text<T>& rhs) -> format requires (!std::is_same_v<T, char8_t>)
	{
		return format {text<char8_t> {lhs}.encode<T>()} | rhs;
	}
	
	template<size_t N>
	friend constexpr auto operator|(const text<T>& lhs, const char16_t (&rhs)[N]) -> format requires (!std::is_same_v<T, char16_t>)
	{
		return format {lhs} | text<char16_t> {rhs}.encode<T>();
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator|(const char16_t (&lhs)[N], const text<T>& rhs) -> format requires (!std::is_same_v<T, char16_t>)
	{
		return format {text<char16_t> {lhs}.encode<T>()} | rhs;
	}

	template<size_t N>
	friend constexpr auto operator|(const text<T>& lhs, const char32_t (&rhs)[N]) -> format requires (!std::is_same_v<T, char32_t>)
	{
		return format {lhs} | text<char32_t> {rhs}.encode<T>();
	}

	template<size_t N>
	// reverse op overloading
	friend constexpr auto operator|(const char32_t (&lhs)[N], const text<T>& rhs) -> format requires (!std::is_same_v<T, char32_t>)
	{
		return format {text<char32_t> {lhs}.encode<T>()} | rhs;
	}

	//|--------------------|
	//| trait::iterable<T> |
	//|--------------------|

	inline constexpr auto begin() const -> iterator { return {&this->c_str()[0]}; }
	inline constexpr auto begin()       -> iterator { return {&this->c_str()[0]}; }

	inline constexpr auto end() const -> iterator { return {&this->c_str()[this->size()]}; }
	inline constexpr auto end()       -> iterator { return {&this->c_str()[this->size()]}; }

	//|---------------------|
	//| trait::printable<T> |
	//|---------------------|

	friend constexpr auto operator<<(std::ostream& os, const text<T>& str) -> std::ostream&
	{
		for (const auto code : str)
		{
			os << code; // see L30
		}
		return os;
	}
};

// https://en.wikipedia.org/wiki/UTF-8
typedef text<char8_t> utf8;
// https://en.wikipedia.org/wiki/UTF-16
typedef text<char16_t> utf16;
// https://en.wikipedia.org/wiki/UTF-32
typedef text<char32_t> utf32;

utf8 operator""_utf(const char8_t* str, size_t len)
{
	return {str};
}

utf16 operator""_utf(const char16_t* str, size_t len)
{
	return {str};
}

utf32 operator""_utf(const char32_t* str, size_t len)
{
	return {str};
}

//|----------|
//| concepts |
//|----------|

namespace model
{
	template<typename T>
	concept text =
	(
		// text_impl
		std::is_same_v<std::remove_cvref_t<T>, utf8>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf16>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf32>
		||
		// text_view
		std::is_same_v<std::remove_cvref_t<T>, utf8::slice>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf16::slice>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf32::slice>
	);

	template<typename T>
	concept text_impl =
	(
		std::is_same_v<std::remove_cvref_t<T>, utf8>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf16>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf32>
	);

	template<typename T>
	concept text_view =
	(
		std::is_same_v<std::remove_cvref_t<T>, utf8::slice>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf16::slice>
		||
		std::is_same_v<std::remove_cvref_t<T>, utf32::slice>
	);
}
