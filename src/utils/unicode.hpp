#pragma once

#include <cstdint>

namespace utils
{
	namespace // private
	{
		struct props
		{
			bool XID_Start : 1 {false};
			bool XID_Continue : 1 {false};
		};

		#include "data/stage1.txt"
		#include "data/stage2.txt"
		#include "data/stage3.txt"
	}

	//|--------------------------------------------------------------|
	//| https://here-be-braces.com/fast-lookup-of-unicode-properties |
	//|--------------------------------------------------------------|

	inline constexpr auto props(const char32_t code) -> decltype(auto)
	{
		return stage3[stage2[(stage1[code >> 8] << 8) | (code & 0xFF)]];
	}
}
