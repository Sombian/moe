#pragma once

#include <map>
#include <set>
#include <vector>
#include <memory>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "core/fs.hpp"
#include "models/str.hpp"
#include "utils/convert.hpp"

constexpr const size_t BLOCK {2*2*2*2*2*2*2*2};
//-----------------------------------------//
typedef std::array<uint16_t, BLOCK> block; //
//-----------------------------------------//
constexpr const size_t LIMIT {0x10FFFF / BLOCK};

struct props
{
	bool XID_Start : 1 {false};
	bool XID_Continue : 1 {false};

	bool operator==(const props&) const = default;
	auto operator<=>(const props&) const = default;
};

namespace
{
	auto UnicodeData(const type::string auto& name) -> std::set<uint32_t>
	{
		static auto UnicodeData // <- cache the file
		{fs::open(u8"codegen/auto/UnicodeData.txt")};

		std::set<uint32_t> result;

		if (UnicodeData)
		{
			std::visit([&](auto&& file)
			{
				// TODO
			},
			*UnicodeData);
		}
		return result;
	}

	template<size_t N>
	// converting constructor
	auto UnicodeData(const char8_t (&name)[N]) -> auto
	{
		return UnicodeData(utf8 {name});
	}

	template<size_t N>
	// converting constructor
	auto UnicodeData(const char16_t (&name)[N]) -> auto
	{
		return UnicodeData(utf16 {name});
	}

	template<size_t N>
	// converting constructor
	auto UnicodeData(const char32_t (&name)[N]) -> auto
	{
		return UnicodeData(utf32 {name});
	}
}

namespace detail
{
	auto CaseFolding(const type::string auto& name) -> std::set<uint32_t>
	{
		static auto CaseFolding // <- cache the file
		{fs::open(u8"codegen/auto/CaseFolding.txt")};

		std::set<uint32_t> result;

		if (CaseFolding)
		{
			std::visit([&](auto&& file)
			{
				// TODO
			},
			*CaseFolding);
		}
		return result;
	}

	template<size_t N>
	// converting constructor
	auto CaseFolding(const char8_t (&name)[N]) -> auto
	{
		return CaseFolding(utf8 {name});
	}

	template<size_t N>
	// converting constructor
	auto CaseFolding(const char16_t (&name)[N]) -> auto
	{
		return CaseFolding(utf16 {name});
	}

	template<size_t N>
	// converting constructor
	auto CaseFolding(const char32_t (&name)[N]) -> auto
	{
		return CaseFolding(utf32 {name});
	}
}

namespace detail
{
	auto CompositionExclusions(const type::string auto& name) -> std::set<uint32_t>
	{
		static auto CompositionExclusions // <- cache the file
		{fs::open(u8"codegen/auto/CompositionExclusions.txt")};

		std::set<uint32_t> result;

		if (CompositionExclusions)
		{
			std::visit([&](auto&& file)
			{
				// TODO

			},
			*CompositionExclusions);
		}
		return result;
	}

	template<size_t N>
	// converting constructor
	auto CompositionExclusions(const char8_t (&name)[N]) -> auto
	{
		return CompositionExclusions(utf8 {name});
	}

	template<size_t N>
	// converting constructor
	auto CompositionExclusions(const char16_t (&name)[N]) -> auto
	{
		return CompositionExclusions(utf16 {name});
	}

	template<size_t N>
	// converting constructor
	auto CompositionExclusions(const char32_t (&name)[N]) -> auto
	{
		return CompositionExclusions(utf32 {name});
	}
}

namespace
{
	auto DerivedCoreProperties(const type::string auto& name) -> std::set<uint32_t>
	{
		static auto DerivedCoreProperties // <- cache the file
		{fs::open(u8"codegen/auto/DerivedCoreProperties.txt")};

		std::set<uint32_t> result;

		if (DerivedCoreProperties)
		{
			std::visit([&](auto&& file)
			{
				for (const auto& line : file.lines())
				{
					if (15 < line.size() && line[0] != '#')
					{
						if (line.find(name, 15) != SIZE_MAX)
						{
							const auto slice {line.split(u8' ')[0]};

							uint32_t foo {0};
							uint32_t bar {0};

							const auto range {slice.split(u8"..")};

							switch (range.size())
							{
								case 1:
								{
									foo = utils::atoi(range[0], 16);
									bar = utils::atoi(range[0], 16);
									break;
								}
								case 2:
								{
									foo = utils::atoi(range[0], 16);
									bar = utils::atoi(range[1], 16);
									break;
								}
							}

							for (auto i {foo}; i <= bar; ++i)
							{
								result.insert(i);
							}
						}
					}
				}
			},
			*DerivedCoreProperties);
		}
		return result;
	}

	template<size_t N>
	// converting constructor
	auto DerivedCoreProperties(const char8_t (&name)[N]) -> auto
	{
		return DerivedCoreProperties(utf8 {name});
	}

	template<size_t N>
	// converting constructor
	auto DerivedCoreProperties(const char16_t (&name)[N]) -> auto
	{
		return DerivedCoreProperties(utf16 {name});
	}

	template<size_t N>
	// converting constructor
	auto DerivedCoreProperties(const char32_t (&name)[N]) -> auto
	{
		return DerivedCoreProperties(utf32 {name});
	}
}

namespace
{
	auto write_stage1(std::vector<uint16_t> stage1, const char* name)
	{
		if (std::ofstream ofs {name})
		{
			ofs << "constexpr const uint16_t stage1[]\n{\n";

			for (size_t i {0}; i < stage1.size(); ++i)
			{
				if (i % 16 == 0)
				{
					ofs << "\t";
				}
				//-----------------------//
				ofs << stage1[i] << ","; //
				//-----------------------//

				if (i % 16 == 15)
				{
					ofs << "\n";
				}
				else // spacing
				{
					ofs << " ";
				}
			}
			ofs << "};\n";
		}
	}

	auto write_stage2(std::vector<block> stage2, const char* name)
	{
		if (std::ofstream ofs {name})
		{
			ofs << "constexpr const uint16_t stage2[]\n{\n";

			for (size_t i {0}; i < stage2.size(); ++i)
			{
				for (size_t j = 0; j < BLOCK; ++j)
				{
					if (j % 16 == 0)
					{
						ofs << "\t";
					}
					//--------------------------//
					ofs << stage2[i][j] << ","; //
					//--------------------------//

					if (j % 16 == 15)
					{
						ofs << "\n";
					}
					else // spacing
					{
						ofs << " ";
					}
				}
			}
			ofs << "};\n";
		}
	}

	auto write_stage3(std::vector<props> stage3, const char* name)
	{
		if (std::ofstream ofs {name})
		{
			ofs << "constexpr const props stage3[]\n{\n";

			for (const auto props : stage3)
			{
				ofs << "\t";
				ofs << "{";
				//-------------------------//
				ofs << props.XID_Start;    //
				ofs << ",";                //
				ofs << " ";                //
				ofs << props.XID_Continue; //
				//-------------------------//
				ofs << "}";
				ofs << ",";
				ofs << "\n";
			}
			ofs << "};\n";
		}
	}
}

namespace setup
{
	auto unicode()
	{
		/*******************/ std::vector<uint16_t> stage1; /*******************/
		/**/ std::vector<block> stage2; std::map<block, uint16_t> stage2map; /**/
		/**/ std::vector<props> stage3; std::map<props, uint16_t> stage3map; /**/
		/***********************************************************************/

		auto POOL {std::make_unique<props[]>(0x10FFFF + 1)};

		//|-----------------------|
		//| step 1. populate pool |
		//|-----------------------|

		for (const auto code : DerivedCoreProperties(u8"XID_Start"))
		{
			POOL[code].XID_Start = true;
		}
		for (const auto code : DerivedCoreProperties(u8"XID_Continue"))
		{
			POOL[code].XID_Continue = true;
		}

		//|-----------------------|
		//| step 2. build schemes |
		//|-----------------------|

		for (auto i {0}; i <= LIMIT; ++i)
		{
			block block {};

			const auto PAGE {i * BLOCK};

			for (auto j {0}; j < BLOCK; ++j)
			{
				const size_t code {PAGE + j};

				block[j] = [&](const props& props)
				{
					const auto [pair, insert]
					{stage3map.try_emplace(props,
					static_cast<uint16_t>(stage3.size()))};
					
					if (insert) // success?
					{
						stage3.emplace_back(props);
					}
					return /* wink */ pair->second;
				}
				(code <= 0x10FFFF ? POOL[code] : props {});
			}

			const auto [pair, insert]
			{stage2map.try_emplace(block,
			static_cast<uint16_t>(stage2.size()))};

			if (insert) // success?
			{
				stage2.emplace_back(block);
			}
			stage1.emplace_back(pair->second);
		}

		//|-----------------------|
		//| step 3. write schemes |
		//|-----------------------|

		write_stage1(stage1, "src/data/stage1.txt");
		std::cout << "[✓] 'stage1.txt'" << '\n';

		write_stage2(stage2, "src/data/stage2.txt");
		std::cout << "[✓] 'stage2.txt'" << '\n';

		write_stage3(stage3, "src/data/stage3.txt");
		std::cout << "[✓] 'stage3.txt'" << '\n';
	}
}
