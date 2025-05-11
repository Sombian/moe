#include <set>

#include "core/fs.hpp"
#include "models/str.hpp"

namespace unicode
{
	[[nodiscard]] static
	auto UnicodeData(const utf8& name) -> std::set<char32_t>
	{
		std::set<char32_t> result;

		if (auto io {fs::open(u8"script/generate/UnicodeData.txt")})
		{
			std::visit([&](auto&& file)
			{
				// TODO
			},
			io.value());
		}
		return result;
	}

	[[nodiscard]] static
	auto CaseFolding(const utf8& name) -> std::set<char32_t>
	{
		std::set<char32_t> result;

		if (auto io {fs::open(u8"script/generate/CaseFolding.txt")})
		{
			std::visit([&](auto&& file)
			{
				// TODO
			},
			io.value());
		}
		return result;
	}

	[[nodiscard]] static
	auto CompositionExclusions(const utf8& name) -> std::set<char32_t>
	{
		std::set<char32_t> result;

		if (auto io {fs::open(u8"script/generate/CompositionExclusions.txt")})
		{
			std::visit([&](auto&& file)
			{
				// TODO
			},
			io.value());
		}
		return result;
	}

	[[nodiscard]] static
	auto DerivedCoreProperties(const utf8& name) -> std::set<char32_t>
	{
		std::set<char32_t> result;

		if (auto io {fs::open(u8"script/generate/DerivedCoreProperties.txt")})
		{
			std::visit([&](auto&& file)
			{
				for (const auto& line : file.lines())
				{
					if (0 < line.size() && line[0] != '#')
					{
						std::cout << line << std::endl;
					}
				}
			},
			io.value());
		}
		return result;
	}
}

auto main() -> int
{
	unicode::DerivedCoreProperties(u8"XID_Start");
}
