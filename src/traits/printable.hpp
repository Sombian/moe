#pragma once

#include <iostream>

namespace traits
{
	template<typename T>
	concept printable = requires(T self)
	{
		std::cout << self; // clear and concise!
	};
}
