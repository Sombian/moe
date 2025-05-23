#include "core/fs.hpp"

#include "lang/lexer.hpp"
#include "lang/parser.hpp"
#include "lang/sentry.hpp"

auto main() -> int
{
	#ifdef _WIN32
		// see [launch.json]
		if (std::getenv("MSVC"))
		{
			//-----<change code page>-----//
			std::system("chcp 65001 > NUL");
			//----------------------------//
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

			sentry
			<
				decltype(file.path),
				decltype(file.data)
			>
			sentry {&parser};

			if (auto&& exe {sentry.pull()})
			{
				exe->print();
			}
		},
		io.value());
	}
}
