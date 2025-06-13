#include <cstdlib>
#include <variant>
#include <iostream>

#include "core/fs.hpp"

#include "lang/1_lexer.hpp"
#include "lang/2_parser.hpp"
#include "lang/3_linter.hpp"

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

			linter
			<
				decltype(file.path),
				decltype(file.data)
			>
			linter {&parser};

			auto exe {linter.pull()};

			#ifndef NDEBUG //-----|
			lang::printer debugger
			{
				std::cout
			};
			exe.visit(debugger);
			#endif //-------------|

			exe.compile(true);
			
		},
		io.value());
	}
	return 0;
}
