#pragma once

#include <cstdint>
#include <iomanip>
#include <iostream>

#include "models/str.hpp"

struct error
{
	utf8 msg;
	//|-<data>-|
	uint16_t x;
	uint16_t y;
	//|--------|

	//|----------------------|
	//| traits::printable<T> |
	//|----------------------|

	friend auto operator<<(std::ostream& os, const error& error) -> std::ostream&
	{
		return
		(
			os
			<<
			"\033[31m" // set color
			<<
			"L"
			<<
			std::setfill('0') << std::setw(2) << error.y
			<<
			":"
			<<
			std::setfill('0') << std::setw(2) << error.x
			<<
			" "
			<<
			error.msg
			<<
			"\033[0m" // reset color
		);
	}
};
