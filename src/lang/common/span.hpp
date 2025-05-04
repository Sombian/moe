#pragma once

#include <vector>
#include <numeric>
#include <cassert>
#include <cstddef>

class span
{
	// column
	class proxy_x
	{
		std::vector<size_t>& data;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		proxy_x(decltype(data) data) : data {data} {}

		//|-----------------|
		//| member function |
		//|-----------------|
		
		operator size_t() const
		{
			return this->data.back();
		}

		auto operator++() -> size_t
		{
			assert(*this < 4095);
			++this->data.back();
			return this->data.back();
		}

		auto operator--() -> size_t
		{
			assert(0 < *this);
			--this->data.back();
			return this->data.back();
		}
	};

	// line
	class proxy_y
	{
		std::vector<size_t>& data;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		proxy_y(decltype(data) data) : data {data} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		operator size_t() const
		{
			return this->data.size();
		}

		auto operator++() -> size_t
		{
			assert(*this < 4095);
			this->data.push_back(0);
			return this->data.size();
		}

		auto operator--() -> size_t
		{
			assert(0 < *this);
			this->data.pop_back();
			return this->data.size();
		}
	};

	std::vector<size_t> data
	{
		0 // <- column
	};

public:

	//|---------------|
	//| the rule of 0 |
	//|---------------|

	span() = default;

	//|-----------------|
	//| member function |
	//|-----------------|

	// offset
	operator size_t() const
	{
		return std::reduce
		(
			this->data.begin(),
			// begin ~ end
			this->data.end()
		);
	}

	[[nodiscard]]
	// x = column
	auto x() const -> size_t
	{
		return this->data.back();
	}

	[[nodiscard]]
	// x = column
	auto x() -> proxy_x
	{
		return {this->data};
	}

	[[nodiscard]]
	// y = line
	auto y() const -> size_t
	{
		return this->data.size();
	}

	[[nodiscard]]
	// y = line
	auto y() -> proxy_y
	{
		return {this->data};
	}
};
