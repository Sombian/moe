#pragma once

#include <cstdint>
#include <compare>

struct span
{
	uint16_t x;
	uint16_t y;

	inline constexpr auto operator<=>(const span& rhs) const
	{
		return
		(
			this->y != rhs.y
			?
			this->y <=> rhs.y
			:
			this->x <=> rhs.x
		);
	}
};

static_assert(span { .x {1}, .y {0} } < span { .x {0}, .y {1} });
static_assert(span { .x {0}, .y {2} } < span { .x {1}, .y {2} });
