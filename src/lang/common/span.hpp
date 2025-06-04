#pragma once

#include <cstdint>
#include <compare>

struct span
{
	uint16_t x;
	uint16_t y;

	constexpr friend
	auto operator<=>
	(
		const span& lhs,
		const span& rhs
	)
	{
		return
		(
			lhs.y != rhs.y
			?
			lhs.y <=> rhs.y
			:
			lhs.x <=> rhs.x
		);
	}
};

static_assert(span { .x {1}, .y {0} } < span { .x {0}, .y {1} });
static_assert(span { .x {0}, .y {2} } < span { .x {1}, .y {2} });
