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
		typename T,
		typename U
	>
	requires requires(T a, U b)
	{
		{ a < b } -> std::convertible_to<bool>;
		{ a > b } -> std::convertible_to<bool>;
		{ b < a } -> std::convertible_to<bool>;
		{ b > a } -> std::convertible_to<bool>;
	}
	inline constexpr auto cmp(const T a, const U b) -> ordering
	{
		//|------------------------------|
		//| (a < b) -> ordering::LESS    |
		//| (a = b) -> ordering::EQUAL   |
		//| (a > b) -> ordering::GREATER |
		//|------------------------------|
		return static_cast<ordering>((a > b) - (a < b));
	}
}
