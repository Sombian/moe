#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <type_traits>

#include "models/str.hpp"

namespace utils
{
	namespace // private
	{
		enum prefix : uint8_t
		{
			NIL = 0 | 0x0, // '�' -> substr(0)
			POS = 1 | 0x1, // '+' -> substr(1)
			NEG = 2 | 0x1, // '-' -> substr(1)
		};

		constexpr auto const TBL
		{
			[]
			{
				std::array<uint8_t, 256> impl {255};

				for (uint8_t i {'0'}; i <= '9'; ++i)
				{
					impl[i] = i - '0' + 0x0;
				}
				for (uint8_t i {'a'}; i <= 'z'; ++i)
				{
					impl[i] = i - 'a' + 0xA;
				}
				for (uint8_t i {'A'}; i <= 'Z'; ++i)
				{
					impl[i] = i - 'A' + 0xA;
				}
				return impl; // 1 to 1 digit mapping
			}
			()
		};
	}

	//|------------------|
	//| string to number |
	//|------------------|

	inline constexpr auto stoi(const model::text auto& str, const uint8_t radix = 10) -> long
	{
		assert(2 <= radix && radix <= 36);

		long out {0};

		const auto pre
		{
			[&]
			{
				switch (str[0])
				{
					case '+': { return prefix::POS; }
					case '-': { return prefix::NEG; }
					default : { return prefix::NIL; }
				}
			}
			()
		};

		for (const auto code : str.substr(pre & 0x1))
		{
			out = (out * radix) + TBL[code];
		}
		
		switch (pre)
		{
			case prefix::NIL: return +out;
			case prefix::POS: return +out;
			case prefix::NEG: return -out;
		}
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto stoi(const char8_t (&str)[N], const uint8_t radix = 10) -> auto
	{
		return stoi(utf8 {str}, radix);
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto stoi(const char16_t (&str)[N], const uint8_t radix = 10) -> auto
	{
		return stoi(utf16 {str}, radix);
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto stoi(const char32_t (&str)[N], const uint8_t radix = 10) -> auto
	{
		return stoi(utf32 {str}, radix);
	}

	//|-----------------|
	//| string to float |
	//|-----------------|

	inline constexpr auto stof(const model::text auto& str) -> double
	{
		auto it {str.begin()}; auto ie {str.end()};

		double nth {0};
		double out {0};

		const auto pre
		{
			[&]
			{
				switch (str[0])
				{
					case '+': { ++it; return prefix::POS; }
					case '-': { ++it; return prefix::NEG; }
					default : {       return prefix::NIL; }
				}
			}
			()
		};

		for (; it != ie; ++it)
		{
			const auto code {*it};
			
			if (code == '.' && nth == 0)
			{
				nth += 10;
				continue;
			}
			if ('0' <= code && code <= '9')
			{
				if (nth == 0)
				{
					out = out * 10 + (code - '0');
				}
				else // decimal
				{
					out = out + (code - '0') / nth;
				}
				nth *= 10;
				continue;
			}
			break;
		}

		switch (pre)
		{
			case prefix::NIL: return +out;
			case prefix::POS: return +out;
			case prefix::NEG: return -out;
		}
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto stof(const char8_t (&str)[N]) -> auto
	{
		return stof(utf8 {str});
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto stof(const char16_t (&str)[N]) -> auto
	{
		return stof(utf16 {str});
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto stof(const char32_t (&str)[N]) -> auto
	{
		return stof(utf32 {str});
	}
}
