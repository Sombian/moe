#pragma once

#include <type_traits>

namespace trait
{
	template<class T>
	concept rule_of_5 =
	(
		std::is_copy_constructible_v<T>
		&&
		std::is_move_constructible_v<T>
		&&
		std::is_copy_assignable_v<T>
		&&
		std::is_move_assignable_v<T>
		&&
		std::is_destructible_v<T>
	);
}

//|---------------|
//| chore methods |
//|---------------|

#define COPY_CALL(T) friend constexpr void copy(const T& from, T& dest) noexcept
#define SWAP_CALL(T) friend constexpr void swap(T& from, T& dest) noexcept

//|-------------|
//| constructor |
//|-------------|

#define COPY_CONSTRUCTOR(T) constexpr T(const T& other) noexcept
#define MOVE_CONSTRUCTOR(T) constexpr T(T&& other) noexcept

//|------------|
//| assignment |
//|------------|

#define COPY_ASSIGNMENT(T) inline constexpr auto operator=(const T& rhs) noexcept -> T&
#define MOVE_ASSIGNMENT(T) inline constexpr auto operator=(T&& rhs) noexcept -> T&
