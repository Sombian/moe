#include <variant>

#include "core/fs.hpp"

#include "lang/lexer.hpp"
#include "lang/parser.hpp"

auto main() -> int
{
	utf8 path {u8"./samples/main.moe"};

	#ifdef _MSC_VER
	{
		#ifdef _WIN32
		//|-----<change code page>-----|
		std::system("chcp 65001 > NUL");
		//|----------------------------|
		#endif//WIN32
	}
	#endif//MSC_VER

	if (auto io {fs::open(path)})
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

			//|----<fontend>----|
			//| lexer -> parser |
			//|-----------------|

			parser.pull().compile();
		},
		io.value());
	}
	return 0;
}
