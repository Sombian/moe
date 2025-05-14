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

	if (auto io {fs::open(u8"sample/main.moe")})
	{
		std::visit([&](auto&& file)
		{
			lexer
			<
				decltype(file.path),
				decltype(file.data)
			>
			lexer {file};
			// lexer.print();

			parser
			<
				decltype(file.path),
				decltype(file.data)
			>
			parser {lexer};
			// parser.print();

			if (const auto result {parser.pull()})
			{
				for (const auto& decl : result->body)
				{
					//---------------------------//
					const auto ptr {decl.get()}; // <- get raw ptr
					//---------------------------//

					if (const auto* var {dynamic_cast<lang::_var*>(ptr)})
					{
						std::cout << "variable:" << var->name << std::endl;
						continue;
					}
					if (const auto* fun {dynamic_cast<lang::_fun*>(ptr)})
					{
						std::cout << "function:" << fun->name << std::endl;
						continue;
					}
				}
			}
		},
		io.value());
	}
}
