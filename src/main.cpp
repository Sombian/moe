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
		std::visit([&](auto&& file)
		{
			lexer
			<
				decltype(file.path),
				decltype(file.data)
			>
			lexer {file};
			lexer.print();

			parser
			<
				decltype(file.path),
				decltype(file.data)
			>
			parser {lexer};
			parser.print();

			std::visit([&](auto&& result)
			{
				typedef std::decay_t<decltype(result)> T;

				//----------------|
				// error count: 0 |
				//----------------|
				if constexpr (std::is_same_v<T, program>)
				{
					for (const auto& decl : result.body)
					{
						//---------------------------//
						const auto ptr {decl.get()}; // <- get raw ptr
						//---------------------------//

						if (const auto* var {dynamic_cast<lang::_var*>(ptr)})
						{
							std::cout << "variable:" << var->name << std::endl;
						}
						if (const auto* fun {dynamic_cast<lang::_fun*>(ptr)})
						{
							std::cout << "function:" << fun->name << std::endl;
						}
					}
				}
				//----------------|
				// error count: N |
				//----------------|
				if constexpr (std::is_same_v<T, error>)
				{
					std::cout << result << std::endl;
				}
			},
			parser.pull());
		},
		sys.value());
	}

	return 0;
}
