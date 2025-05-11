#pragma once

#include <array>

namespace utils
{
	namespace
	{
		struct props
		{
			bool XID_Start    : 1;
			bool XID_Continue : 1;
		};

		#include "data/stage1.txt"
		#include "data/stage2.txt"
		#include "data/stage3.txt"
	}

	//|--------------------------------------------------------------|
	//| https://here-be-braces.com/fast-lookup-of-unicode-properties |
	//|--------------------------------------------------------------|

	auto props(const char32_t code) -> props
	{
		return stage3[stage2[stage1[code >> 8] + (code & 0xFF)]];
	}
}
