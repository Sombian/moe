#include <cstdlib>

#include "core/fs.hpp"

#include "lang/lexer.hpp"
#include "lang/linter.hpp"
#include "lang/parser.hpp"

auto main() -> int
{
	// see [launch.json]
	if (std::getenv("MSVC"))
	{
		#ifdef _WIN32
		//|-----<change code page>-----|
		std::system("chcp 65001 > NUL");
		//|----------------------------|
		#endif//WIN32
	}

	if (auto io {fs::open(u8"sample/main.moe")})
	{
		std::visit([&](auto&& file)
		{
			//|-----------------------|
			//| [source] → [frontend] |
			//|-----------------------|

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

			//|------------------------|
			//| [frontend] → [backend] |
			//|------------------------|

			if (auto& exe {linter.pull()})
			{
				
			}
		},
		io.value());
	}
	return 0;
}
