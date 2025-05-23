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
struct error
{
	//|---<safe ptr>---|
	fs::file<A, B>* file;
	//|----------------|
	utf8 data;
	span span;

public:

	//|-----------------|
	//| member function |
	//|-----------------|

	auto operator==(const error& type) const -> bool = default;
	auto operator!=(const error& type) const -> bool = default;

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
			error.file->path
			<<
			"("
			<<
			std::setfill('0') << std::setw(2) << error.span.y + 0
			<<
			","
			<<
			std::setfill('0') << std::setw(2) << error.span.x + 1
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
