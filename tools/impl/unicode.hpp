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

namespace // private
{
	struct props
	{
		bool XID_Start : 1 {false};
		bool XID_Continue : 1 {false};

		inline constexpr bool
		operator==(const props&) const = default;
		
		inline constexpr auto
		operator<=>(const props&) const = default;
	};

	inline constexpr const size_t BLOCK {2*2*2*2*2*2*2*2};
	//-----------------------------------------//
	typedef std::array<uint16_t, BLOCK> block; //
	//-----------------------------------------//
	inline constexpr const size_t LIMIT {0x10FFFF / BLOCK};
}

namespace // private
{
	inline constexpr auto DerivedCoreProperties(const model::text auto& name) -> std::set<uint32_t>
	{
		static auto DerivedCoreProperties // cache the file
		{fs::open(u8"tools/data/DerivedCoreProperties.txt")};

		std::set<uint32_t> result;

		if (DerivedCoreProperties)
		{
			std::visit([&](auto&& file)
			{
				for (const auto& line : file.lines())
				{
					if (!line.empty() && line[0] != '#')
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
									foo = utils::stoi(range[0], 16);
									bar = utils::stoi(range[0], 16);
									break;
								}
								case 2:
								{
									foo = utils::stoi(range[0], 16);
									bar = utils::stoi(range[1], 16);
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
	inline constexpr auto DerivedCoreProperties(const char8_t (&str)[N]) -> auto
	{
		return DerivedCoreProperties(utf8 {str});
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto DerivedCoreProperties(const char16_t (&str)[N]) -> auto
	{
		return DerivedCoreProperties(utf16 {str});
	}

	template<size_t N>
	// converting constructor
	inline constexpr auto DerivedCoreProperties(const char32_t (&str)[N]) -> auto
	{
		return DerivedCoreProperties(utf32 {str});
	}
}

namespace // private
{
	inline /*Ი︵𐑼*/ auto write_stage1(std::vector<uint16_t>& stage1, const char* name)
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

	inline /*Ი︵𐑼*/ auto write_stage2(std::vector<block>& stage2, const char* name)
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

	inline /*Ი︵𐑼*/ auto write_stage3(std::vector<props>& stage3, const char* name)
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
	inline /*Ი︵𐑼*/ auto unicode()
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

		for (size_t i {0}; i <= LIMIT; ++i)
		{
			block block {};

			const auto PAGE {i * BLOCK};

			for (size_t j {0}; j < BLOCK; ++j)
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
