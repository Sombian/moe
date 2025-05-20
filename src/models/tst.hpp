#pragma once

#include <vector>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <optional>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

#include "models/str.hpp"

#include "traits/rule_of_5.hpp"

#include "utils/ordering.hpp"

//|--------------------------|
//| TST(Ternary Search Tree) |
//|--------------------------|

template<typename T>
class tst
{
	struct node
	{
		const char32_t code;
		// storage
		[[no_unique_address]]
		std::optional<T> data;
		// children
		node* left {nullptr};
		node* middle {nullptr};
		node* right {nullptr};

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		~node()
		{
			delete this->left;
			delete this->middle;
			delete this->right;
		}

		//|-----------------|
		//| member function |
		//|-----------------|

		[[nodiscard]] static
		auto repair(node* a) -> node*
		{
			auto balance {node::factor(a)};

			// left heavy
			if (+1 < balance)
			{
				// LL case
				if (a->left->code < a->code)
				{
					return a = node::rotate_r(a);
				}
				// LR case
				if (a->code < a->left->code)
				{
					a->left = node::rotate_l(a->left);
					return a = node::rotate_r(a);
				}
			}
			// right heavy
			if (balance < -1)
			{
				// RR case
				if (a->code < a->right->code)
				{
					return a = node::rotate_l(a);
				}
				// RL case
				if (a->right->code < a->code)
				{
					a->right = node::rotate_r(a->right);
					return a = node::rotate_l(a);
				}
			}
			return a;
		}

	private:

		[[nodiscard]] static
		auto factor(node* a) -> int8_t
		{
			if (a == nullptr) { return 0; }
			auto left {node::height(a->left)};
			auto right {node::height(a->right)};
			return left - right; // delta
		}

		[[nodiscard]] static
		auto height(node* a) -> int8_t
		{
			if (a == nullptr) { return 0; }
			auto left {node::height(a->left)};
			auto right {node::height(a->right)};
			return std::max(left, right) + 1;
		}

		[[nodiscard]] static
		auto rotate_l(node* a) -> node*
		{
			//|--------|-------|-------|-------|
			//|        | A     |       |   B   |
			//|        |  ╲    |       |  ╱ ╲  |
			//| BEFORE |   B   | AFTER | A   C |
			//|        |    ╲  |       |       |
			//|        |     C |       |       |
			//|--------|-------|-------|-------|

			auto b {a->right};
			auto c {b->left};

			b->left = a;
			a->right = c;

			return b;
		}

		[[nodiscard]] static
		auto rotate_r(node* a) -> node*
		{
			//|--------|-------|-------|-------|
			//|        |     A |       |   B   |
			//|        |    ╱  |       |  ╱ ╲  |
			//| BEFORE |   B   | AFTER | C   A |
			//|        |  ╱    |       |       |
			//|        | C     |       |       |
			//|--------|-------|-------|-------|

			auto b {a->left};
			auto c {b->right};

			b->right = a;
			a->left = c;

			return b;
		}
	};

	node* root; // unique_ptr..?

public:

	~tst()
	{
		delete this->root; // ok
	}

	//|---------------|
	//| the rule of 5 |
	//|---------------|

	template<type::string S>
	tst(std::initializer_list<std::pair<const S&, T>> args = {})
	{
		for (const auto& [first, second] : args)
		{
			this->operator[](first) = second;
		}
	}

	// converting constructor support
	tst(std::initializer_list<std::pair<const utf8, T>> args = {})
	{
		for (const auto& [first, second] : args)
		{
			this->operator[](first) = second;
		}
	}

	// converting constructor support
	tst(std::initializer_list<std::pair<const utf16, T>> args = {})
	{
		for (const auto& [first, second] : args)
		{
			this->operator[](first) = second;
		}
	}

	// converting constructor support
	tst(std::initializer_list<std::pair<const utf32, T>> args = {})
	{
		for (const auto& [first, second] : args)
		{
			this->operator[](first) = second;
		}
	}

	COPY_CONSTRUCTOR(tst)
	{
		if (this != &other)
		{
			// TODO
		}
	}

	MOVE_CONSTRUCTOR(tst)
	{
		if (this != &other)
		{
			// TODO
		}
	}

	COPY_ASSIGNMENT(tst)
	{
		if (this != &rhs)
		{
			// TODO
		}
		return *this;
	}

	MOVE_ASSIGNMENT(tst)
	{
		if (this != &rhs)
		{
			// TODO
		}
		return *this;
	}

	//|-----------------|
	//| member function |
	//|-----------------|

private:

	template
	<
		typename S
	>
	requires
	(
		std::is_same_v<S, tst<T>&>
		||
		std::is_same_v<S, tst<T> const&>
	)
	class cursor
	{
		S src;
		node* ptr;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		cursor
		(
			decltype(src) src,
			decltype(ptr) ptr
		)
		: src {src}, ptr {ptr} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		auto reset()
		{
			this->ptr = nullptr;
		}

		auto get() const -> std::optional<T> requires
		(
			std::is_class_v<T> ? !std::is_empty_v<T> : true
		)
		{
			return this->ptr ? this->ptr->data : std::nullopt;
		}

		auto is_root() const -> bool
		{
			return this->ptr == nullptr;
		}

		auto is_leaf() const -> bool
		{
			return ptr && !ptr->left && !ptr->middle && !ptr->right;
		}

		auto is_child() const -> bool
		{
			return this->ptr != nullptr;
		}

		auto is_parent() const -> bool
		{
			return ptr && (ptr->left || ptr->middle || ptr->right);
		}

		// incremental search
		auto operator[](const char32_t idx) -> bool
		{
			node* ptr
			{
				!this->ptr
				?
				this->src.root
				:
				this->ptr->middle
			};

			while (true)
			{
				if (ptr == nullptr)
				{
					return false;
				}
				switch (utils::cmp(idx, ptr->code))
				{
					case utils::ordering::LESS:
					{
						ptr = ptr->left;
						break;
					}
					case utils::ordering::EQUAL:
					{
						goto exit;
					}
					case utils::ordering::GREATER:
					{
						ptr = ptr->right;
						break;
					}
				}
			}
			exit:
			return this->ptr = ptr;
		}
	};

public:

	auto view() const -> cursor<decltype(*this)>
	{
		return {*this, nullptr};
	}

private:

	template
	<
		typename S,
		type::string U
	>
	requires
	(
		std::is_same_v<S, tst<T>&>
		||
		std::is_same_v<S, tst<T> const&>
	)
	class proxy
	{
		S src;
		const U str;

	public:

		//|---------------|
		//| the rule of 0 |
		//|---------------|

		proxy
		(
			decltype(src) src,
			decltype(str) str
		)
		: src {src}, str {str} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		operator bool() const&& requires
		(
			std::is_class_v<T> ? std::is_empty_v<T> : false
		)
		{
			auto ptr {this->src.root};

			size_t nth {0};

			for (const auto code : this->str)
			{
				while (true)
				{
					if (ptr == nullptr)
					{
						return false;
					}
					switch (utils::cmp(code, ptr->code))
					{
						case utils::ordering::LESS:
						{
							ptr = ptr->left;
							break;
						}
						case utils::ordering::EQUAL:
						{
							if (nth < this->str.length() - 1)
							{
								ptr = ptr->middle;
							}
							goto exit;
						}
						case utils::ordering::GREATER:
						{
							ptr = ptr->right;
							break;
						}
					}
				}
				exit:
				++nth;
			}
			return ptr != nullptr;
		}

		operator std::optional<T>() const&& requires
		(
			std::is_class_v<T> ? !std::is_empty_v<T> : true
		)
		{
			auto ptr {this->src.root};

			size_t nth {0};

			for (const auto code : this->str)
			{
				while (true)
				{
					if (ptr == nullptr)
					{
						return std::nullopt;
					}
					switch (utils::cmp(code, ptr->code))
					{
						case utils::ordering::LESS:
						{
							ptr = ptr->left;
							break;
						}
						case utils::ordering::EQUAL:
						{
							if (nth < this->str.length() - 1)
							{
								ptr = ptr->middle;
							}
							goto exit;
						}
						case utils::ordering::GREATER:
						{
							ptr = ptr->right;
							break;
						}
					}
				}
				exit:
				++nth;
			}
			// return the value if found
			return ptr ? ptr->data : std::nullopt;
		}

		auto operator=(const T& value)&& -> proxy& requires
		(
			!std::is_const_v<std::remove_reference_t<S>>
		)
		{
			auto ptr {&this->src.root};

			size_t nth {0};

			std::vector<node**> stack;

			for (const auto code : this->str)
			{
				while (true)
				{
					// remember path
					stack.push_back(ptr);

					if ((*ptr) == nullptr)
					{
						(*ptr) = new node(code);
					}
					switch (utils::cmp(code, (*ptr)->code))
					{
						case utils::ordering::LESS:
						{
							ptr = &((*ptr)->left);
							break;
						}
						case utils::ordering::EQUAL:
						{
							if (nth < this->str.length() - 1)
							{
								ptr = &((*ptr)->middle);
							}
							goto exit;
						}
						case utils::ordering::GREATER:
						{
							ptr = &((*ptr)->right);
							break;
						}
					}
				}
				exit:
				++nth;
			}
			// set the node value
			(*ptr)->data = value;

			// bubble up to the root
			while (!stack.empty())
			{
				auto ptr {stack.back()};
				stack.pop_back(); // POP!
				*ptr = node::repair(*ptr);
			}
			return *this; // chain
		}
	};

public:

	//|----------|
	//| lhs[str] | -> const proxy
	//|----------|

	template<type::string S>
	auto operator[](const S& str) const -> proxy<decltype(*this), decltype(str)>
	{
		return {*this, str};
	}

	// converting constructor support
	auto operator[](const utf8 str) const -> proxy<decltype(*this), utf8>
	{
		return {*this, str};
	}

	// converting constructor support
	auto operator[](const utf16 str) const -> proxy<decltype(*this), utf16>
	{
		return {*this, str};
	}

	// converting constructor support
	auto operator[](const utf32 str) const -> proxy<decltype(*this), utf32>
	{
		return {*this, str};
	}

	//|----------|
	//| lhs[str] | -> proxy
	//|----------|

	template<type::string S>
	auto operator[](const S& str) -> proxy<decltype(*this), decltype(str)>
	{
		return {*this, str};
	}

	// converting constructor support
	auto operator[](const utf8 str) -> proxy<decltype(*this), utf8>
	{
		return {*this, str};
	}

	// converting constructor support
	auto operator[](const utf16 str) -> proxy<decltype(*this), utf16>
	{
		return {*this, str};
	}

	// converting constructor support
	auto operator[](const utf32 str) -> proxy<decltype(*this), utf32>
	{
		return {*this, str};
	}

	//|-----------|
	//| lhs & rhs |
	//|-----------|

	auto operator&(const tst<T>& rhs) const -> tst<T>
	{
		// TODO
	}

	//|-----------|
	//| lhs | rhs |
	//|-----------|

	auto operator|(const tst<T>& rhs) const -> tst<T>
	{
		// TODO
	}

	//|-----------|
	//| lhs + rhs |
	//|-----------|

	auto operator+(const tst<T>& rhs) const -> tst<T>
	{
		// TODO
	}

	//|-----------|
	//| lhs - rhs |
	//|-----------|

	auto operator-(const tst<T>& rhs) const -> tst<T>
	{
		// TODO
	}
};
