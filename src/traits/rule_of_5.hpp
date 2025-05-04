#pragma once

#include <type_traits>

namespace traits
{
	template<typename T>
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

//|-------------|
//| constructor |
//|-------------|

#define COPY_CONSTRUCTOR($type) $type(const $type& other) noexcept
#define MOVE_CONSTRUCTOR($type) $type($type&& other) noexcept

//|------------|
//| assignment |
//|------------|

#define COPY_ASSIGNMENT($type) auto operator=(const $type& rhs) noexcept -> $type&
#define MOVE_ASSIGNMENT($type) auto operator=($type&& rhs) noexcept -> $type&
//|--------|
//| chores |
//|--------|

#define COPY_CALL($type) friend auto copy(const $type& from, $type& dest) noexcept
#define SWAP_CALL($type) friend auto swap($type& from, $type& dest) noexcept
