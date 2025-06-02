#pragma once

#include <array>
#include <memory>
#include <cassert>
#include <cstdint>
#include <utility>
#include <variant>
#include <optional>
#include <functional>

#include "lang/lexer.hpp"

#include "./common/ast.hpp"
#include "./common/error.hpp"
#include "./common/token.hpp"

template
<
	type::string A,
	type::string B
>
class parser
{
	lexer<A, B>* lexer;
	
	uint16_t x;
	uint16_t y;
	//|---------<buffer>---------|
	decltype(lexer->pull()) buffer;
	//|--------------------------|

	#define E(value) error<A, B> \
	{                            \
		this->x,                 \
		this->y,                 \
		*this,                   \
		value,                   \
	}                            \

	//|---------------<maybe>--------------|
	typedef std::optional<token<A, B>> maybe;
	//|------------------------------------|

	auto peek() -> maybe
	{
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<A, B>>)
			{
				return /**/ arg /**/;
			}
			if constexpr (std::is_same_v<T, error<A, B>>)
			{
				return std::nullopt;
			}
			return std::nullopt;
		},
		this->buffer);
	}

	auto next() -> maybe
	{
		// step 1. update buffer
		this->buffer = this->lexer->pull();

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<A, B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|

				return /**/ arg /**/;
			}
			if constexpr (std::is_same_v<T, error<A, B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|

				return std::nullopt;
			}
			return std::nullopt;
		},
		this->buffer);
	}

	auto peek(const atom type) -> bool
	{
		return std::visit([&](auto&& arg) -> bool
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<A, B>>)
			{
				return arg == type;
			}
			if constexpr (std::is_same_v<T, error<A, B>>)
			{
				return false;
			}
			return false;
		},
		this->buffer);
	}

	auto next(const atom type) -> bool
	{
		// step 1. update buffer
		this->buffer = this->lexer->pull();

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> bool
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<A, B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|

				return arg == type;
			}
			if constexpr (std::is_same_v<T, error<A, B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|

				return false;
			}
			return false;
		},
		this->buffer);
	}

public:

	parser
	(
		decltype(lexer) lexer
	)
	: lexer {lexer}, buffer {this->lexer->pull()}
	{
		std::visit([&](auto&& arg)
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (!std::is_same_v<T, eof>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|
			}
		},
		this->buffer);
	}

	//|-----------------|
	//| member function |
	//|-----------------|
	
	operator fs::file<A, B>*()
	{
		return *this->lexer;
	}

	auto pull() -> std::optional<program>
	{
		program exe;
		try
		{
			while (true)
			{
				if (auto out {this->decl_t()})
				{
					//|-------------<insert>-------------|
					exe.ast.emplace_back(std::move(*out));
					//|----------------------------------|
					continue;
				}
				if (auto out {this->stmt_t()})
				{
					//|-------------<insert>-------------|
					exe.ast.emplace_back(std::move(*out));
					//|----------------------------------|
					continue;
				}
				break;
			}
			// if not EOF
			if (this->peek())
			{
				throw E(u8"[parser] unknown token");
			}
		}
		catch (error<A, B>& error)
		{
			#ifndef NDEBUG //---------|
			std::cout << error << '\n';
			#endif //-----------------|
			return std::nullopt;
		}
		return exe;
	}

private:

	//|--------------|
	//| declarations |
	//|--------------|

	auto decl_t() -> std::optional<decl>
	{
		if (const auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::LET:
				{
					return this->decl_var(false);
				}
				case atom::LET$:
				{
					return this->decl_var(true);
				}
				case atom::FUN:
				{
					return this->decl_fun(false);
				}
				case atom::FUN$:
				{
					return this->decl_fun(true);
				}
			}
		}
		return std::nullopt;
	}

	auto decl_var(const bool is_const) -> decltype(this->decl_t())
	{
		$var node;
		
		node.x = this->x;
		node.y = this->y;

		this->next();

		//|-------<update>------|
		node.is_const = is_const;
		//|---------------------|

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			node.name = std::move(tkn.data);
			//|----------------------------|
		}
		else throw E(u8"[parser] N/A <sym>");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ':'");

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			node.type = std::move(tkn.data);
			//|----------------------------|
		}
		else throw E(u8"[parser] N/A <sym>");

		if (!node.is_const)
		{
			if (this->peek(atom::ASSIGN))
			{
				this->next();

				//|--------------<catch>--------------|
				node.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{ throw E(u8"[parser] N/A expr"); });
				//|-----------------------------------|
			}
			// else throw E(u8"[parser] must init const var");
		}
		else // let! name
		{
			if (this->peek(atom::ASSIGN))
			{
				this->next();

				//|--------------<catch>--------------|
				node.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{ throw E(u8"[parser] N/A expr"); });
				//|-----------------------------------|
			}
			else throw E(u8"[parser] must init const var");
		}

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto decl_fun(const bool is_pure) -> decltype(this->decl_t())
	{
		$fun node;

		node.x = this->x;
		node.y = this->y;
		
		this->next();

		//|------<update>-----|
		node.is_pure = is_pure;
		//|-------------------|

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			node.name = std::move(tkn.data);
			//|----------------------------|
		}
		else throw E(u8"[parser] N/A <sym>");
	
		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		start:
		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();
			
			$var args;

			//|----------<update>----------|
			args.name = std::move(tkn.data);
			//|----------------------------|

			if (this->peek(atom::COLON))
			{
				this->next();
			}
			else throw E(u8"[parser] N/A ':'");

			if (this->peek(atom::SYMBOL))
			{
				//|----------<copy>----------|
				const auto tkn {*this->peek()};
				//|--------------------------|

				this->next();

				//|----------<update>----------|
				args.type = std::move(tkn.data);
				//|----------------------------|
			}
			else throw E(u8"[parser] N/A <sym>");

			if (this->peek(atom::ASSIGN))
			{
				this->next();

				//|--------------<catch>--------------|
				args.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{ throw E(u8"[parser] N/A expr"); });
				//|-----------------------------------|
			}
			// else throw E(u8"[parser] N/A '='");

			//|--------------<insert>--------------|
			node.args.emplace_back(std::move(args));
			//|------------------------------------|
			
			if (this->peek(atom::COMMA))
			{
				this->next();
				goto start;
			}
			// else throw E(u8"[parser] N/A ','");
		}

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ':'");

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			node.type = std::move(tkn.data);
			//|----------------------------|
		}
		else throw E(u8"[parser] N/A <sym>");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|--------------<insert>--------------|
				node.body.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->decl_t()})
			{
				//|--------------<insert>--------------|
				node.body.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|--------------<insert>--------------|
				node.body.emplace_back(std::move(*out));
				//|------------------------------------|
	
				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	//|------------|
	//| statements |
	//|------------|

	auto stmt_t() -> std::optional<stmt>
	{
		if (const auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::IF:
				{
					return this->stmt_if();
				}
				case atom::FOR:
				{
					return this->stmt_for();
				}
				case atom::MATCH:
				{
					return this->stmt_match();
				}
				case atom::WHILE:
				{
					return this->stmt_while();
				}
				case atom::BREAK:
				{
					return this->stmt_break();
				}
				case atom::RETURN:
				{
					return this->stmt_return();
				}
				case atom::CONTINUE:
				{
					return this->stmt_continue();
				}
			}
		}
		return std::nullopt;
	}

	auto stmt_if() -> decltype(this->stmt_t())
	{
		$if node;

		node.x = this->x;
		node.y = this->y;
		
		this->next();

		expr expr;
		body body;
		
		else_if:
		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
			expr = {*this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); })};
		//|-----------------------------------|

		//|--------------<insert>--------------|
		node.cases.emplace_back(std::move(expr));
		//|------------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if_block:
		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|-----------<insert>-----------|
				body.emplace_back(std::move(*out));
				//|------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|-----------<insert>-----------|
				body.emplace_back(std::move(*out));
				//|------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		//|--------------<insert>--------------|
		node.block.emplace_back(std::move(body));
		//|------------------------------------|

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		if (this->peek(atom::ELSE))
		{
			if (this->next(atom::IF))
			{
				this->next();
				goto else_if;
			}
			goto if_block;
		}

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_for() -> decltype(this->stmt_t())
	{
		$for node;

		node.x = this->x;
		node.y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
		node.setup = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		//|--------------<catch>--------------|
		node.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		//|--------------<catch>--------------|
		node.after = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|--------------<insert>--------------|
				node.block.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|--------------<insert>--------------|
				node.block.emplace_back(std::move(*out));
				//|------------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_match() -> decltype(this->stmt_t())
	{
		$match node;

		node.x = this->x;
		node.y = this->y;
		
		this->next();

		expr expr;
		body body;

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
		node.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		if (this->peek(atom::CASE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A 'case'");

		case_block:
		//|--------------<catch>--------------|
			expr = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		//|--------------<insert>--------------|
		node.cases.emplace_back(std::move(expr));
		//|------------------------------------|

		else_block:
		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ':'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|-----------<insert>-----------|
				body.emplace_back(std::move(*out));
				//|------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|-----------<insert>-----------|
				body.emplace_back(std::move(*out));
				//|------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		//|--------------<insert>--------------|
		node.block.emplace_back(std::move(body));
		//|------------------------------------|

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		// check if theres more...
		if (this->peek(atom::CASE))
		{
			this->next();
			goto case_block;
		}
		// check if theres more...
		if (this->peek(atom::ELSE))
		{
			this->next();
			goto else_block;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_while() -> decltype(this->stmt_t())
	{
		$while node;

		node.x = this->x;
		node.y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
		node.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|--------------<insert>--------------|
				node.block.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|--------------<insert>--------------|
				node.block.emplace_back(std::move(*out));
				//|------------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");
	
		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_break() -> decltype(this->stmt_t())
	{
		$break node;

		node.x = this->x;
		node.y = this->y;
		
		this->next();

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			node.label = std::move(tkn.data);
			//|----------------------------|
		}

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_return() -> decltype(this->stmt_t())
	{
		$return node;

		node.x = this->x;
		node.y = this->y;

		this->next();

		if (auto out {this->expr_t()})
		{
			//|--------<update>---------|
			node.value = std::move(*out);
			//|-------------------------|
		}

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_continue() -> decltype(this->stmt_t())
	{
		$continue node;
		
		node.x = this->x;
		node.y = this->y;
		
		this->next();

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			node.label = std::move(tkn.data);
			//|----------------------------|
		}

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	//|-------------|
	//| expressions |
	//|-------------|

	auto expr_t() -> std::optional<expr>
	{
		// pratt parser impl
		std::function<std::optional<expr>(uint8_t)> impl
		{
			[&](const uint8_t mbp) -> std::optional<expr>
			{
				auto lhs
				{
					[&] -> std::optional<expr>
					{
						if (auto tkn {this->peek()})
						{
							// handle prefix
							if (lang::is_l(*tkn))
							{
								$unary node;
								
								node.x = this->x;
								node.y = this->y;

								this->next();

								//|--------------<catch>--------------|
								std::optional rhs {impl(69).or_else([&]
									-> decltype(this->expr_t())
								{ throw E(u8"[parser] N/A expr");}) };
								//|-----------------------------------|

								node.op = lang::to_l(*tkn);
								node.rhs = std::move(*rhs);

								return std::make_unique /*(wrap)*/
								<decltype(node)>(std::move(node));
							}
							// handle primary
							if (auto out {this->expr_group()})
							{
								return std::move(out);
							}
							if (auto out {this->expr_symbol()})
							{
								return std::move(out);
							}
							if (auto out {this->expr_literal()})
							{
								return std::move(out);
							}
							// invalid expr
							return std::nullopt;
						}
						// error or eof
						return std::nullopt;
					}
					()
				};

				if (lhs != std::nullopt)
				{
					while (auto tkn {this->peek()})
					{
						// handle infix
						if (lang::is_i(*tkn))
						{
							auto [lbp, rbp]
							{
								[&]() -> std::array<uint8_t, 2>
								{
									switch (tkn->type)
									{
										case atom::ASSIGN:
										case atom::ADD_EQ:
										case atom::SUB_EQ:
										case atom::MUL_EQ:
										case atom::DIV_EQ:
										case atom::MOD_EQ:
										case atom::POW_EQ:
										{
											return {1, 2};
										}
										case atom::NIL:
										{
											return {3, 4};
										}
										case atom::B_OR:
										case atom::L_OR:
										{
											return {5, 6};
										}
										case atom::B_AND:
										case atom::L_AND:
										{
											return {7, 8};
										}
										case atom::EQ:
										case atom::NE:
										{
											return {9, 10};
										}
										case atom::LT:
										case atom::GT:
										case atom::LTE:
										case atom::GTE:
										{
											return {11, 12};
										}
										case atom::SHL:
										case atom::SHR:
										{
											return {13, 14};
										}
										case atom::ADD:
										case atom::SUB:
										{
											return {15, 16};
										}
										case atom::MUL:
										case atom::DIV:
										case atom::MOD:
										{
											return {17, 18};
										}
										case atom::POW:
										{
											return {19, 18};
										}
										default:
										{
											assert(!!!"error");
											std::unreachable();
										}
									}
								}
								()
							};
							if (lbp < mbp)
							{
								break;
							}

							$binary node;

							node.x = this->x;
							node.y = this->y;
							
							this->next();

							//|--------------<catch>--------------|
							std::optional rhs {impl(rbp).or_else([&]
								-> decltype(this->expr_t())
							{ throw E(u8"[parser] N/A expr"); })};
							//|-----------------------------------|

							node.op = lang::to_i(*tkn);
							node.lhs = std::move(*lhs);
							node.rhs = std::move(*rhs);

							lhs = std::make_unique /*(wrap)*/
							<decltype(node)>(std::move(node));
							continue;
						}

						// handle postfix
						if (lang::is_r(*tkn))
						{
							$access node;

							node.x = this->x;
							node.y = this->y;
							
							this->next();

							if (this->peek(atom::SYMBOL))
							{
								//|----------<copy>----------|
								const auto tkn {*this->peek()};
								//|--------------------------|

								this->next();

								//|----------<update>----------|
								node.name = std::move(tkn.data);
								//|----------------------------|
							}
							else throw E(u8"[parser] N/A <sym>");

							node.expr = std::move(*lhs);
							node.type = lang::to_r(*tkn);
							
							lhs = std::make_unique /*(wrap)*/
							<decltype(node)>(std::move(node));
							continue;
						}

						// handle function
						if (tkn == atom::L_PAREN)
						{
							$call node;

							node.x = this->x;
							node.y = this->y;

							this->next();

							//|--------<update>--------|
							node.call = std::move(*lhs);
							//|------------------------|

							start:
							if (auto ast {this->expr_t()})
							{
								//|--------------<insert>--------------|
								node.args.emplace_back(std::move(*ast));
								//|------------------------------------|

								if (this->peek(atom::COMMA))
								{
									this->next();
									goto start;
								}
								// else throw E(u8"[parser] N/A ','");
							}

							if (this->peek(atom::R_PAREN))
							{
								this->next();
							}
							else throw E(u8"[parser] N/A ')'");

							lhs = std::make_unique /*(wrap)*/
							<decltype(node)>(std::move(node));
							continue;
						}
						break;
					}
				}
				return lhs;
			}
		};
		return impl(0);
	}

	auto expr_group() -> decltype(this->expr_t())
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::L_PAREN:
				{
					$group node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();

					//|--------------<catch>--------------|
					node.expr = *this->expr_t().or_else([&]
						-> decltype(this->expr_t())
					{ throw E(u8"[parser] N/A expr"); });
					//|-----------------------------------|

					if (this->peek(atom::R_PAREN))
					{
						this->next();
					}
					else throw E(u8"[parser] N/A ')'");

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
			}
		}
		return std::nullopt;
	}

	auto expr_symbol() -> decltype(this->expr_t())
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::SYMBOL:
				{
					$symbol node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();
					
					//|----------<update>----------|
					node.name = std::move(tkn->data);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
			}
		}
		return std::nullopt;
	}

	auto expr_literal() -> decltype(this->expr_t())
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::TRUE:
				case atom::FALSE:
				{
					$literal node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::BOOL);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case atom::CHAR:
				{
					$literal node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::CODE);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case atom::TEXT:
				{
					$literal node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::UTF8);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case atom::DEC:
				{
					$literal node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::F32);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case atom::INT:
				case atom::BIN:
				case atom::OCT:
				case atom::HEX:
				{
					$literal node;

					node.x = this->x;
					node.y = this->y;
					
					this->next();

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::I32);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
			}
		}
		return std::nullopt;
	}

	#undef E
};
