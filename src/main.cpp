#include <cstdlib>

#include "core/fs.hpp"

#include "lang/lexer.hpp"
#include "lang/linter.hpp"
#include "lang/parser.hpp"

auto main() -> int
{
	#ifdef _WIN32
	// see [launch.json]
	if (std::getenv("MSVC"))
	{
		//|-----<change code page>-----|
		std::system("chcp 65001 > NUL");
		//|----------------------------|
	}
	#endif

	if (auto io {fs::open(u8"sample/main.moe")})
	{
		std::visit([&](auto&& file)
		{
			lexer
			<
				decltype(file.path),
				decltype(file.data)
			>
			lexer {&file};

			parser
			<
				decltype(file.path),
				decltype(file.data)
			>
			parser {&lexer};

			linter
			<
				decltype(file.path),
				decltype(file.data)
			>
			linter {&parser};

			if (auto&& exe {linter.pull()})
			{
				// exe->print();
			}
		},
		io.value());
	}
	return 0;
}
