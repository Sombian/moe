#pragma once

#include <array>
#include <vector>
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
	//|--<safe ref>---|
	lexer<A, B>& lexer;
	//|---------------|
	uint16_t x;
	uint16_t y;
	//|---------<buffer>---------|
	decltype(lexer.pull()) buffer;
	//|--------------------------|

	#define E($value) error \
	{                       \
		.msg                \
		{                   \
			$value          \
		},                  \
		.x                  \
		{                   \
			this->x         \
		},                  \
		.y                  \
		{                   \
			this->y         \
		},                  \
	}                       \

	//|--------------<maybe>-------------|
	typedef std::optional<token<B>> maybe;
	//|----------------------------------|

	auto peek() -> maybe
	{
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<B>>)
			{
				return /**/ arg /**/;
			}
			if constexpr (std::is_same_v<T, error>)
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
		this->buffer = this->lexer.pull();

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return /**/ arg /**/;
			}
			if constexpr (std::is_same_v<T, error>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return std::nullopt;
			}
			return std::nullopt;
		},
		this->buffer);
	}

	auto peek(const lexeme type) -> bool
	{
		return std::visit([&](auto&& arg) -> bool
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<B>>)
			{
				return arg == type;
			}
			if constexpr (std::is_same_v<T, error>)
			{
				return false;
			}
			return false;
		},
		this->buffer);
	}

	auto next(const lexeme type) -> bool
	{
		// step 1. update buffer
		this->buffer = this->lexer.pull();

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> bool
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return arg == type;
			}
			if constexpr (std::is_same_v<T, error>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

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
	: lexer {lexer}, buffer {lexer.pull()} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	auto pull() -> std::optional<program>
	{
		program exe;
		try
		{
			while (true)
			{
				if (auto out {this->stmt_t()})
				{
					//|-------------<insert>-------------|
					exe.ast.emplace_back(std::move(*out));
					//|----------------------------------|
					continue;
				}
				if (auto out {this->decl_t()})
				{
					//|-------------<insert>-------------|
					exe.ast.emplace_back(std::move(*out));
					//|----------------------------------|
					continue;
				}
				break;
			}
		}
		catch (error& error)
		{
			#ifndef NDEBUG //-------------|
			std::cout << error << std::endl;
			#endif //---------------------|
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
				case lexeme::LET:
				{
					return this->decl_var(false);
				}
				case lexeme::LET$:
				{
					return this->decl_var(true);
				}
				case lexeme::FUN:
				{
					return this->decl_fun(false);
				}
				case lexeme::FUN$:
				{
					return this->decl_fun(true);
				}
			}
		}
		return std::nullopt;
	}

	auto decl_var(const bool is_const) -> decltype(this->decl_t())
	{
		this->next();
		
		lang::$var node;

		//|-------<update>------|
		node.is_const = is_const;
		//|---------------------|

		if (this->peek(lexeme::SYMBOL))
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

		if (this->peek(lexeme::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ':'");

		if (this->peek(lexeme::SYMBOL))
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
			if (this->peek(lexeme::ASSIGN))
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
			if (this->peek(lexeme::ASSIGN))
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

		if (this->peek(lexeme::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto decl_fun(const bool is_pure) -> decltype(this->decl_t())
	{
		this->next();

		lang::$fun node;

		//|------<update>-----|
		node.is_pure = is_pure;
		//|-------------------|

		if (this->peek(lexeme::SYMBOL))
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
	
		if (this->peek(lexeme::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		start:
		if (this->peek(lexeme::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();
			
			lang::$var args;

			//|----------<update>----------|
			args.name = std::move(tkn.data);
			//|----------------------------|

			if (this->peek(lexeme::COLON))
			{
				this->next();
			}
			else throw E(u8"[parser] N/A ':'");

			if (this->peek(lexeme::SYMBOL))
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

			if (this->peek(lexeme::ASSIGN))
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
			
			if (this->peek(lexeme::COMMA))
			{
				this->next();
				goto start;
			}
			// else throw E(u8"[parser] N/A ','");
		}

		if (this->peek(lexeme::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(lexeme::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ':'");

		if (this->peek(lexeme::SYMBOL))
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

		if (this->peek(lexeme::L_BRACE))
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
	
				if (this->peek(lexeme::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(lexeme::R_BRACE))
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
				case lexeme::IF:
				{
					return this->stmt_if();
				}
				case lexeme::MATCH:
				{
					return this->stmt_match();
				}
				case lexeme::FOR:
				{
					return this->stmt_for();
				}
				case lexeme::WHILE:
				{
					return this->stmt_while();
				}
				case lexeme::BREAK:
				{
					return this->stmt_break();
				}
				case lexeme::RETURN:
				{
					return this->stmt_return();
				}
				case lexeme::CONTINUE:
				{
					return this->stmt_continue();
				}
			}
		}
		return std::nullopt;
	}

	auto stmt_if() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$if node;

		expr expr;
		body body;
		
		else_if:
		if (this->peek(lexeme::L_PAREN))
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

		if (this->peek(lexeme::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if_block:
		if (this->peek(lexeme::L_BRACE))
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

				if (this->peek(lexeme::S_COLON))
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

		if (this->peek(lexeme::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		if (this->peek(lexeme::ELSE))
		{
			if (this->next(lexeme::IF))
			{
				this->next();
				goto else_if;
			}
			goto if_block;
		}

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_match() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$match node;

		expr expr;
		body body;

		if (this->peek(lexeme::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
		node.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(lexeme::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(lexeme::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '{'");

		if (this->peek(lexeme::CASE))
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
		if (this->peek(lexeme::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ':'");

		if (this->peek(lexeme::L_BRACE))
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

				if (this->peek(lexeme::S_COLON))
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

		if (this->peek(lexeme::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		// check if theres more...
		if (this->peek(lexeme::CASE))
		{
			this->next();
			goto case_block;
		}
		// check if theres more...
		if (this->peek(lexeme::ELSE))
		{
			this->next();
			goto else_block;
		}

		if (this->peek(lexeme::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_for() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$for node;

		if (this->peek(lexeme::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
		node.setup = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(lexeme::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		//|--------------<catch>--------------|
		node.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(lexeme::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		//|--------------<catch>--------------|
		node.after = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(lexeme::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(lexeme::L_BRACE))
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

				if (this->peek(lexeme::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(lexeme::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_while() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$while node;

		if (this->peek(lexeme::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '('");

		//|--------------<catch>--------------|
		node.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{ throw E(u8"[parser] N/A expr"); });
		//|-----------------------------------|

		if (this->peek(lexeme::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ')'");

		if (this->peek(lexeme::L_BRACE))
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

				if (this->peek(lexeme::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] N/A ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(lexeme::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A '}'");
	
		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_break() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$break node;

		if (this->peek(lexeme::SYMBOL))
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
		this->next();

		lang::$return node;

		if (auto out {this->expr_t()})
		{
			//|--------<update>---------|
			node.value = std::move(*out);
			//|-------------------------|
		}

		if (this->peek(lexeme::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] N/A ';'");

		return std::make_unique /*(wrap)*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_continue() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$continue node;

		if (this->peek(lexeme::SYMBOL))
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
							if (op::is_l(*tkn))
							{
								this->next();
								
								//|--------------<catch>--------------|
								std::optional rhs {impl(69).or_else([&]
									-> decltype(this->expr_t())
								{ throw E(u8"[parser] N/A expr");}) };
								//|-----------------------------------|

								lang::$unary node
								{
									.lhs {((op::l(*tkn)))},
									.rhs {std::move(*rhs)},
								};

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
						if (op::is_i(*tkn))
						{
							auto [lbp, rbp]
							{
								[&]() -> std::array<uint8_t, 2>
								{
									switch (tkn->type)
									{
										case lexeme::ASSIGN:
										case lexeme::ASSIGN_ADD:
										case lexeme::ASSIGN_SUB:
										case lexeme::ASSIGN_MUL:
										case lexeme::ASSIGN_DIV:
										case lexeme::ASSIGN_MOD:
										case lexeme::ASSIGN_POW:
										{
											return {1, 2};
										}
										case lexeme::NIL:
										{
											return {3, 4};
										}
										case lexeme::B_OR:
										case lexeme::L_OR:
										{
											return {5, 6};
										}
										case lexeme::B_AND:
										case lexeme::L_AND:
										{
											return {7, 8};
										}
										case lexeme::EQ:
										case lexeme::NE:
										{
											return {9, 10};
										}
										case lexeme::LT:
										case lexeme::GT:
										case lexeme::LTE:
										case lexeme::GTE:
										{
											return {11, 12};
										}
										case lexeme::SHL:
										case lexeme::SHR:
										{
											return {13, 14};
										}
										case lexeme::ADD:
										case lexeme::SUB:
										{
											return {15, 16};
										}
										case lexeme::MUL:
										case lexeme::DIV:
										case lexeme::MOD:
										{
											return {17, 18};
										}
										case lexeme::POW:
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
							this->next();

							//|--------------<catch>--------------|
							std::optional rhs {impl(rbp).or_else([&]
								-> decltype(this->expr_t())
							{ throw E(u8"[parser] N/A expr"); })};
							//|-----------------------------------|

							lang::$binary node
							{
								.lhs {std::move(*lhs)},
								.mhs {((op::i(*tkn)))},
								.rhs {std::move(*rhs)},
							};

							lhs = std::make_unique /*(wrap)*/
							<decltype(node)>(std::move(node));
							continue;
						}

						// handle postfix
						if (op::is_r(*tkn))
						{
								this->next();

								lang::$access node
								{
									.expr {std::move(*lhs)},
									.type {((op::r(*tkn)))},
								};

								if (this->peek(lexeme::SYMBOL))
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
								
								lhs = std::make_unique /*(wrap)*/
								<decltype(node)>(std::move(node));
								continue;
						}

						// handle function
						if (tkn == lexeme::L_PAREN)
						{
							this->next();
						
							lang::$call node;

							//|--------<update>--------|
							node.call = std::move(*lhs);
							//|------------------------|

							start:
							if (auto ast {this->expr_t()})
							{
								//|--------------<insert>--------------|
								node.args.emplace_back(std::move(*ast));
								//|------------------------------------|

								if (this->peek(lexeme::COMMA))
								{
									this->next();
									goto start;
								}
								// else throw E(u8"[parser] N/A ','");
							}

							if (this->peek(lexeme::R_PAREN))
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
				case lexeme::L_PAREN:
				{
					this->next();
					
					lang::$group node;

					//|--------------<catch>--------------|
					node.expr = *this->expr_t().or_else([&]
						-> decltype(this->expr_t())
					{ throw E(u8"[parser] N/A expr"); });
					//|-----------------------------------|

					if (this->peek(lexeme::R_PAREN))
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
				case lexeme::SYMBOL:
				{
					this->next();

					lang::$symbol node;
					
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
				case lexeme::TRUE:
				case lexeme::FALSE:
				{
					this->next();
					
					lang::$literal node;

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::BOOL);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::CHAR:
				{
					this->next();

					lang::$literal node;

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::CODE);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::TEXT:
				{
					this->next();

					lang::$literal node;

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::UTF8);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::DEC:
				{
					this->next();

					lang::$literal node;

					//|----------<update>----------|
					node.data = std::move(tkn->data);
					node.type = std::move(data::F32);
					//|----------------------------|

					return std::make_unique /*(wrap)*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::INT:
				case lexeme::BIN:
				case lexeme::OCT:
				case lexeme::HEX:
				{
					this->next();

					lang::$literal node;

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
