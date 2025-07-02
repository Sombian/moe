#pragma once

#include <deque>
#include <cstddef>
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
		
		inline /*Ი︵𐑼*/ operator size_t() const
		{
			return this->data.back();
		}

		inline /*Ი︵𐑼*/ auto operator++() -> size_t
		{
			++this->data.back();
			return this->data.back();
		}

		inline /*Ი︵𐑼*/ auto operator--() -> size_t
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

		inline /*Ი︵𐑼*/ operator size_t() const
		{
			return this->data.size();
		}

		inline /*Ი︵𐑼*/ auto operator++() -> size_t
		{
			this->data.push_back(0);
			return this->data.size();
		}

		inline /*Ი︵𐑼*/ auto operator--() -> size_t
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

	inline constexpr auto o() const -> size_t
	{
		return std::reduce
		(
			this->data.begin(),
			this->data.end(),
			0 // from RE:0
		);
	}

	inline /*Ი︵𐑼*/ auto x() const -> size_t
	{
		return this->data.back();
	}

	inline /*Ი︵𐑼*/ auto x()       -> proxy_x
	{
		return {this->data};
	}

	inline /*Ი︵𐑼*/ auto y() const -> size_t
	{
		return this->data.size();
	}

	inline /*Ი︵𐑼*/ auto y()       -> proxy_y
	{
		return {this->data};
	}
};
