#pragma once

#include <cstdint>
#include <concepts>

namespace utils
{
	enum class ordering : int8_t
	{
		LESS    = -1, // historic convension
		EQUAL   =  0, // historic convension
		GREATER = +1, // historic convension
	};

	template
	<
		typename T1,
		typename T2
	>
	requires requires(T1 a, T2 b)
	{
		{ a < b } -> std::convertible_to<bool>;
		{ a > b } -> std::convertible_to<bool>;
		{ b < a } -> std::convertible_to<bool>;
		{ b > a } -> std::convertible_to<bool>;
	}
	constexpr auto cmp(T1 a, T2 b) -> ordering
	{
		return static_cast<ordering>((a > b) - (a < b));
	}
}
