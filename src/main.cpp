#include <variant>
#include <iostream>

#include "core/fs.hpp"

#include "lang/lexer.hpp"
#include "lang/parser.hpp"

#include "lang/backend/analyzer.hpp"
#include "lang/backend/compiler.hpp"

auto main() -> int
{
	utf8 path {u8"./tests/main.moe"};

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

			//|---------------------|
			//| frontend -> backend |
			//|---------------------|

			auto exe {parser.pull()};

			for (auto& _ : exe.lint)
			{
				std::cout << _ << '\n';
			}
			compiler().compile(exe);
		},
		io.value());
	}
	return 0;
}
