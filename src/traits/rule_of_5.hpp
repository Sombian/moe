#pragma once

#include <type_traits>

namespace trait
{
	template<class T>
	concept rule_of_5 =
	(
		// construtor
		std::is_copy_constructible_v<T>
		&&
		std::is_move_constructible_v<T>
		&&
		// destructor
		std::is_destructible_v<T>
		&&
		// assignment
		std::is_copy_assignable_v<T>
		&&
		std::is_move_assignable_v<T>
	);
}

//|---------------|
//| chore methods |
//|---------------|

#define COPY_CALL($type) friend constexpr void copy(const $type& from, $type& dest) noexcept
#define SWAP_CALL($type) friend constexpr void swap($type& from, $type& dest) noexcept

//|-------------|
//| constructor |
//|-------------|

#define COPY_CONSTRUCTOR($type) constexpr $type(const $type& other) noexcept
#define MOVE_CONSTRUCTOR($type) constexpr $type($type&& other) noexcept

//|------------|
//| assignment |
//|------------|

#define COPY_ASSIGNMENT($type) inline constexpr auto operator=(const $type& rhs) noexcept -> $type&
#define MOVE_ASSIGNMENT($type) inline constexpr auto operator=($type&& rhs) noexcept -> $type&
