#include <filesystem>

#include "./tools/unicode.hpp"

auto main() -> int
{
	//------------------[adjust CWD]------------------//
	const auto CWD {std::filesystem::current_path()}; //
	std::filesystem::current_path(CWD.parent_path()); //
	//------------------------------------------------//

	setup::unicode();
}
