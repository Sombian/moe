#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "models/str.hpp"

namespace utils
{
	// ASCII to integer
	inline constexpr auto atoi(const type::string auto& str, const uint8_t radix = 10) -> size_t
	{
		assert(2 <= radix && radix <= 36);

		constexpr static const auto TBL
		{
			[]
			{
				std::array<uint8_t, 256> impl {255};

				for (uint8_t i {'0'}; i <= '9'; ++i) { impl[i] = i - '0' + 0x0; }
				for (uint8_t i {'a'}; i <= 'z'; ++i) { impl[i] = i - 'a' + 0xA; }
				for (uint8_t i {'A'}; i <= 'Z'; ++i) { impl[i] = i - 'A' + 0xA; }

				return impl;
			}
			()
		};

		size_t out {0};

		for (const auto code : str)
		{
			if (radix <= TBL[code])
			{
				break;
			}
			out *= radix;
			out += TBL[code];
		}
		return out;
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto atoi(const char8_t (&str)[N], const uint8_t radix = 10) -> auto
	{
		return atoi(utf8 {str}, radix);
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto atoi(const char16_t (&str)[N], const uint8_t radix = 10) -> auto
	{
		return atoi(utf16 {str}, radix);
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto atoi(const char32_t (&str)[N], const uint8_t radix = 10) -> auto
	{
		return atoi(utf32 {str}, radix);
	}

	// integer to ASCII
	inline constexpr auto itoa(int64_t value, const uint8_t radix = 10) -> utf8
	{
		assert(2 <= radix && radix <= 36);

		static constexpr char8_t const TBL[]
		{
			// 0 ~ 9
			u8'0', u8'1',
			u8'2', u8'3',
			u8'4', u8'5',
			u8'6', u8'7',
			u8'8', u8'9',
			// A - Z
			u8'A', u8'B',
			u8'C', u8'D',
			u8'E', u8'F',
			u8'G', u8'H',
			u8'I', u8'J',
			u8'K', u8'L',
			u8'M', u8'N',
			u8'O', u8'P',
			u8'Q', u8'R',
			u8'S', u8'T',
			u8'U', u8'V',
			u8'W', u8'X',
			u8'Y', u8'Z',
		};
		/*------------<data>------------*/
		char8_t buffer[65]; size_t i {64};
		/*------------------------------*/

		// save it before modifying 'value'
		const auto negative {value < 0};

		// short-circuit
		if (value == 0)
		{
			buffer[--i] = u8'0';
		}
		else
		{
			while (0 < value)
			{
				buffer[--i] = TBL[value % radix];
				/*----*/ value /= radix; /*----*/
			}
		}
		if (negative)
		{
			buffer[--i] = u8'-';
		}
		// portion of str
		return {&buffer[i]};
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto itoa(int32_t value, const uint8_t radix = 10) -> auto
	{
		return itoa(value, radix);
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto itoa(int16_t value, const uint8_t radix = 10) -> auto
	{
		return itoa(value, radix);
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto itoa(int8_t value, const uint8_t radix = 10) -> auto
	{
		return itoa(value, radix);
	}

	// unsigned to ASCII
	inline constexpr auto utoa(uint64_t value, const uint8_t radix = 10) -> utf8
	{
		assert(2 <= radix && radix <= 36);

		static constexpr char8_t const TBL[]
		{
			// 0 ~ 9
			u8'0', u8'1',
			u8'2', u8'3',
			u8'4', u8'5',
			u8'6', u8'7',
			u8'8', u8'9',
			// A - Z
			u8'A', u8'B',
			u8'C', u8'D',
			u8'E', u8'F',
			u8'G', u8'H',
			u8'I', u8'J',
			u8'K', u8'L',
			u8'M', u8'N',
			u8'O', u8'P',
			u8'Q', u8'R',
			u8'S', u8'T',
			u8'U', u8'V',
			u8'W', u8'X',
			u8'Y', u8'Z',
		};
		/*------------<data>------------*/
		char8_t buffer[65]; size_t i {64};
		/*------------------------------*/

		if (value == 0)
		{
			buffer[--i] = u8'0';
		}
		else
		{
			while (0 < value)
			{
				buffer[--i] = TBL[value % radix];
				/*----*/ value /= radix; /*----*/
			}
		}
		// portion of str
		return {&buffer[i]};
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto utoa(int32_t value, const uint8_t radix = 10) -> auto
	{
		return utoa(value, radix);
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto utoa(int16_t value, const uint8_t radix = 10) -> auto
	{
		return utoa(value, radix);
	}

	template
	<
		size_t N
	>
	// converting constructor
	inline constexpr auto utoa(int8_t value, const uint8_t radix = 10) -> auto
	{
		return utoa(value, radix);
	}
}
