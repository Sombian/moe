#pragma once

#include <deque>
#include <numeric>

class trail
{
	// column
	class proxy_x
	{
		std::deque<size_t>& data;

	public:

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
			++this->data.back();
			return this->data.back();
		}

		auto operator--() -> size_t
		{
			--this->data.back();
			return this->data.back();
		}
	};

	// line
	class proxy_y
	{
		std::deque<size_t>& data;

	public:

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
			this->data.push_back(0);
			return this->data.size();
		}

		auto operator--() -> size_t
		{
			this->data.pop_back();
			return this->data.size();
		}
	};

	std::deque<size_t> data
	{
		0 // <- column
	};

public:

	trail() = default;

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

	// x = column
	auto x() const -> size_t
	{
		return this->data.back();
	}

	// x = column
	auto x() -> proxy_x
	{
		return {this->data};
	}

	// y = line
	auto y() const -> size_t
	{
		return this->data.size();
	}

	// y = line
	auto y() -> proxy_y
	{
		return {this->data};
	}
};
