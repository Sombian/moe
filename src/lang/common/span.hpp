#pragma once

#include <cstdint>
#include <compare>

struct span
{
	uint16_t x;
	uint16_t y;

	bool operator==(const span&) const = default;
	auto operator<=>(const span&) const = default;
};
