#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "models/str.hpp"

namespace utils
{
	auto number(const type::string auto& str, uint8_t base = 10) -> size_t
	{
		static constexpr const auto TBL
		{
			[]
			{
				std::array<uint8_t, 256> impl {255};

				for (int i = '0'; i <= '9'; ++i) { impl[i] = i - '0' + 0x0; }
				for (int i = 'a'; i <= 'z'; ++i) { impl[i] = i - 'a' + 0xA; }
				for (int i = 'A'; i <= 'Z'; ++i) { impl[i] = i - 'A' + 0xA; }

				return impl; // <- return lookup table
			}
			()
		};

		size_t result {0};

		for (const auto code : str)
		{
			if (base <= TBL[code])
			{
				break;
			}
			result *= base;
			result += TBL[code];
		}
		return result;
	}

	template<size_t N>
	// converting constructor support
	auto number(const char8_t (&str)[N], uint8_t base = 10) -> auto
	{
		return number(utf8 {str}, base);
	}

	template<size_t N>
	// converting constructor support
	auto number(const char16_t (&str)[N], uint8_t base = 10) -> auto
	{
		return number(utf16 {str}, base);
	}

	template<size_t N>
	// converting constructor support
	auto number(const char32_t (&str)[N], uint8_t base = 10) -> auto
	{
		return number(utf32 {str}, base);
	}
}
