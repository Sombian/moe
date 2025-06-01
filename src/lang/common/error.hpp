#pragma once

#include <iomanip>
#include <iostream>

#include "core/fs.hpp"

#include "./span.hpp"

#include "models/str.hpp"

template
<
	type::string A,
	type::string B
>
struct error : public span
{
	//|-----<file>-----|
	fs::file<A, B>* src;
	//|----------------|
	utf8 data;

public:

	error
	(
		decltype(x) x,
		decltype(y) y,
		decltype(src) src,
		decltype(data) data
	)
	:
	span {x, y}, src {src}, data {data} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	friend auto operator==(const error& lhs, const error& rhs) -> bool = default;
	friend auto operator!=(const error& lhs, const error& rhs) -> bool = default;

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
			error.src->path
			<<
			"("
			<<
			std::setfill('0') << std::setw(2) << error.y + 0
			<<
			","
			<<
			std::setfill('0') << std::setw(2) << error.x + 1
			<<
			")"
			<<
			" "
			<<
			error.data
			<<
			"\033[0m" // reset color
		);
	}
};
