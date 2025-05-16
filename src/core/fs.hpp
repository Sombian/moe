#pragma once

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <variant>
#include <fstream>
#include <optional>
#include <iostream>
#include <filesystem>

#include "models/str.hpp"

namespace fs
{
	template
	<
		type::string A,
		type::string B
	>
	struct file
	{
		A path;
		B data;

		auto lines() -> auto
		{
			return this->data.split(U'\n');
		}
	};

	template<type::string T>
	auto open(const T& path) -> std::optional<std::variant<file<T, utf8>, file<T, utf16>, file<T, utf32>>>
	{
		#ifndef NDEBUG //-------------|
		std::cout << path << std::endl;
		#endif //---------------------|
	
		auto sys {std::filesystem::path(path.c_str())};

		if (std::ifstream ifs {sys, std::ios::binary})
		{
			//|-------------------------|
			//| step 1. detect encoding |
			//|-------------------------|

			enum encoding : uint8_t
			{
				UTF8_STD = (0 << 4) | 0,
				UTF8_BOM = (1 << 4) | 3,
				UTF16_BE = (2 << 4) | 2,
				UTF16_LE = (3 << 4) | 2,
				UTF32_BE = (4 << 4) | 4,
				UTF32_LE = (5 << 4) | 4,
			}
			const BOM = [&] -> encoding
			{
				char buffer[4]
				{
					0, // <- clear
					0, // <- clear
					0, // <- clear
					0, // <- clear
				};

				ifs.seekg(0, std::ios::beg);
				ifs.read(&buffer[0], 4);

				if
				(
					buffer[0] == '\x00'
					&&
					buffer[1] == '\x00'
					&&
					buffer[2] == '\xFE'
					&&
					buffer[3] == '\xFF'
				)
				{
					return UTF32_BE;
				}
				if
				(
					buffer[0] == '\xFF'
					&&
					buffer[1] == '\xFE'
					&&
					buffer[2] == '\x00'
					&&
					buffer[3] == '\x00'
				)
				{
					return UTF32_LE;
				}
				if
				(
					buffer[0] == '\xFE'
					&&
					buffer[1] == '\xFF'
				)
				{
					return UTF16_BE;
				}
				if
				(
					buffer[0] == '\xFF'
					&&
					buffer[1] == '\xFE'
				)
				{
					return UTF16_LE;
				}
				if
				(
					buffer[0] == '\xEF'
					&&
					buffer[1] == '\xBB'
					&&
					buffer[2] == '\xBF'
				)
				{
					return UTF8_BOM;
				}
				return UTF8_STD;
			}
			();

			const auto off {BOM & 0xF};

			//|------------------------|
			//| step 2. calculate size |
			//|------------------------|

			// to the BOM
			ifs.seekg(off, std::ios::beg);
			auto size {ifs.tellg()};
			// to the end
			ifs.seekg(0, std::ios::end);
			size = ifs.tellg() - size;
			// to the BOM
			ifs.clear(); // <- important
			ifs.seekg(off, std::ios::beg);

			//|------------------------|
			//| step 3. read file data |
			//|------------------------|

			const auto write_native
			{
				[&]<typename unit>(text<unit>& str)
				{
					auto size {0};
					unit code {0};

					for (auto ptr {str.c_str()}; !ifs.eof(); ++size)
					{
						if (ifs.read(reinterpret_cast<char*>(&code), sizeof(unit)); code == '\r')
						{
							if (ifs.read(reinterpret_cast<char*>(&code), sizeof(unit)); code != '\n')
							{
								assert(!!!"error");
								std::unreachable();
							}
						}
						ptr[size] = code;
					}
					// update size
					str.size(size - 1);
				}
			};

			const auto write_foreign
			{
				[&]<typename unit>(text<unit>& str)
				{
					auto size {0};
					unit code {0};

					for (auto ptr {str.c_str()}; !ifs.eof(); ++size)
					{
						if (ifs.read(reinterpret_cast<char*>(&code), sizeof(unit)); (code = std::byteswap(code)) == '\r')
						{
							if (ifs.read(reinterpret_cast<char*>(&code), sizeof(unit)); (code = std::byteswap(code)) != '\n')
							{
								assert(!!!"error");
								std::unreachable();
							}
						}
						ptr[size] = code;
					}
					// update size
					str.size(size - 1);
				}
			};

			switch (BOM)
			{
				case UTF8_STD:
				case UTF8_BOM:
				{
					typedef char8_t unit;
					
					text<unit> data {size / sizeof(unit)};

					write_native(data); // no need to swap

					return file<T, decltype(data)>{std::move(path), std::move(data)};
				}
				case UTF16_BE:
				{
					typedef char16_t unit;

					text<unit> data {size / sizeof(unit)};

					if (std::endian::native == std::endian::big)
					{
						write_native(data); // write as it is
					}
					else
					{
						write_foreign(data); // write as flips
					}
					return file<T, decltype(data)>{std::move(path), std::move(data)};
				}
				case UTF16_LE:
				{
					typedef char16_t unit;

					text<unit> data {size / sizeof(unit)};

					if (std::endian::native == std::endian::little)
					{
						write_native(data); // write as it is
					}
					else
					{
						write_foreign(data); // write as flips
					}
					return file<T, decltype(data)>{std::move(path), std::move(data)};
				}
				case UTF32_BE:
				{
					typedef char32_t unit;

					text<unit> data {size / sizeof(unit)};

					if (std::endian::native == std::endian::big)
					{
						write_native(data); // write as it is
					}
					else
					{
						write_foreign(data); // write as flips
					}
					return file<T, decltype(data)>{std::move(path), std::move(data)};
				}
				case UTF32_LE:
				{
					typedef char32_t unit;

					text<unit> data {size / sizeof(unit)};
					
					if (std::endian::native == std::endian::little)
					{
						write_native(data); // write as it is
					}
					else
					{
						write_foreign(data); // write as flips
					}
					return file<T, decltype(data)>{std::move(path), std::move(data)};
				}
			}
		}
		return std::nullopt;
	}

	template<size_t N>
	// converting constructor support
	auto open(const char8_t (&path)[N])
	{
		return open(utf8 {path});
	}

	template<size_t N>
	// converting constructor support
	auto open(const char16_t (&path)[N])
	{
		return open(utf16 {path});
	}

	template<size_t N>
	// converting constructor support
	auto open(const char32_t (&path)[N])
	{
		return open(utf32 {path});
	}
}
