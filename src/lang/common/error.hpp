#pragma once

#include <iomanip>
#include <iostream>

#include "core/fs.hpp"

#include "./span.hpp"

#include "models/str.hpp"

template
<
	model::text A,
	model::text B
>
struct error : public span
{
	//|-----<file>-----|
	fs::file<A, B>* src;
	//|----------------|
	utf8 msg;

public:

	error
	(
		decltype(x) x,
		decltype(y) y,
		decltype(src) src,
		decltype(msg) msg
	)
	:
	span {x, y}, src {src}, msg {msg} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	friend constexpr auto operator==(const error& lhs, const error& rhs) -> bool = default;

	friend constexpr auto operator!=(const error& lhs, const error& rhs) -> bool = default;

	//|---------------------|
	//| trait::printable<T> |
	//|---------------------|

	friend constexpr auto operator<<(std::ostream& os, const error& error) -> std::ostream&
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
			":"
			<<
			std::setfill('0') << std::setw(2) << error.x + 1
			<<
			")"
			<<
			" "
			<<
			error.msg
			<<
			"\033[0m" // reset color
		);
	}
};
