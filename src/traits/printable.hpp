#pragma once

#include <iostream>

namespace traits
{
	template<class T>
	concept printable = requires(T self)
	{
		std::cout << self; // this works :3
	};
}
