#include <cstdlib>
#include <variant>

#include "core/fs.hpp"

#include "lang/lexer.hpp"
#include "lang/parser.hpp"

auto main() -> int
{
	#ifdef _WIN32
		// see [launch.json]
		if (std::getenv("MSVC"))
		{
			//-----[change code page]-----//
			std::system("chcp 65001 > NUL");
			//----------------------------//
		}
	#endif

	if (auto sys {fs::open(u8"sample/main.moe")})
	{
		std::visit([](auto&& arg)
		{
			std::cout << arg.path << std::endl;
			std::cout << arg.data << std::endl;

			lexer
			<
				decltype(arg.path),
				decltype(arg.data)
			>
			lexer {arg};
			lexer.print();

			parser
			<
				decltype(arg.path),
				decltype(arg.data)
			>
			parser {lexer};
			parser.print();
		},
		sys.value());
	}

	return 0;
}
