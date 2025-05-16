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

namespace op
{
	template<typename B>
	auto is_l(const token<B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			operator_l(macro)
			#undef macro
			default:
			{
				return false;
			}
		}
	}

	template<typename B>
	auto l(const token<B>& tkn) -> op_l
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_l::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_l(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}

	template<typename B>
	auto is_i(const token<B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			operator_i(macro)
			#undef macro
			default:
			{
				return false;
			}
		}
	}

	template<typename B>
	auto i(const token<B>& tkn) -> op_i
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_i::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_i(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}

	template<typename B>
	auto is_r(const token<B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			operator_r(macro)
			#undef macro
			default:
			{
				return false;
			}
		}
	}

	template<typename B>
	auto r(const token<B>& tkn) -> op_r
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_r::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_r(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}
}

template
<
	type::string A,
	type::string B
>
class parser
{
	lexer<A, B>& lexer;

	//|-<data>-|
	uint16_t x;
	uint16_t y;
	//|--------|

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

	//----------------<buffer>----------------|
	std::vector<decltype(lexer.pull())> buffer;
	//----------------------------------------|

	auto peek() -> maybe
	{
		if (const auto ptr {std::get_if<token<B>>(&this->buffer.back())})
		{ /* fuck you GCC */ return *ptr; } else { return std::nullopt; }
	}

	auto next() -> maybe
	{
		auto result {this->lexer.pull()};

		// step 1. accumulate buffer
		this->buffer.push_back(result);

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, error>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return std::nullopt;
			}
			if constexpr (std::is_same_v<T, token<B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return /**/ arg /**/;
			}
			return std::nullopt;
		},
		this->buffer.back());
	}

public:

	//|---------------|
	//| the rule of 0 |
	//|---------------|

	parser(decltype(lexer) lexer) : lexer {lexer}
	{
		// on construct
		this->next();
		// pull a token
	}

	//|-----------------|
	//| member function |
	//|-----------------|

	auto pull() -> std::optional<program>
	{
		// full ast
		program exe;
		try
		{
			while (true)
			{
				if (auto out {this->stmt_t()})
				{
					//|-------------<INSERT>-------------|
					exe.ast.emplace_back(std::move(*out));
					//|----------------------------------|
					continue;
				}
				if (auto out {this->decl_t()})
				{
					//|-------------<INSERT>-------------|
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
		// update metadata
		node.is_const = is_const;

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next();
			
			//|-----<assign>-----|
			node.name = tkn->data;
			//|------------------|
		}
		else throw E(u8"[parser] expects <sym>");

		if (auto tkn {this->peek()}; tkn == lexeme::COLON)
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next();
			
			//|-----<assign>-----|
			node.type = tkn->data;
			//|------------------|
		}
		else throw E(u8"[parser] expects <sym>");

		if (!node.is_const)
		{
			if (auto tkn {this->peek()}; tkn == lexeme::ASSIGN)
			{
				this->next();

				//|--------------<catch>--------------|
				node.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{throw E(u8"[parser] expects expr");});
				//------------------------------------|
			}
			// else throw E(u8"[parser] must init const var");

			if (auto tkn {this->peek()}; tkn == lexeme::S_COLON)
			{
				this->next();
			}
			else throw E(u8"[parser] expects ';'");
		}
		else // let! name
		{
			if (auto tkn {this->peek()}; tkn == lexeme::ASSIGN)
			{
				this->next();
				
				//|--------------<catch>--------------|
				node.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{throw E(u8"[parser] expects expr");});
				//------------------------------------|
			}
			else throw E(u8"[parser] must init const var");

			if (auto tkn {this->peek()}; tkn == lexeme::S_COLON)
			{
				this->next();
			}
			else throw E(u8"[parser] expects ':'");
		}

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto decl_fun(const bool is_pure) -> decltype(this->decl_t())
	{
		this->next();

		lang::$fun node;
		// update metadata
		node.is_pure = is_pure;

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next(); node.name = tkn->data;
		}
		else throw E(u8"[parser] expects <sym>");
	
		if (auto tkn {this->peek()}; tkn == lexeme::L_PAREN)
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		start:
		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next();
			
			lang::$var args;
			// update metadata
			args.name = tkn->data;

			if (tkn = this->peek(); tkn == lexeme::COLON)
			{
				this->next();
			}
			else throw E(u8"[parser] expects ':'");

			if (tkn = this->peek(); tkn == lexeme::SYMBOL)
			{
				this->next(); args.type = tkn->data;
			}
			else throw E(u8"[parser] expects <sym>");

			if (tkn = this->peek(); tkn == lexeme::ASSIGN)
			{
				this->next();
				
				//|--------------<catch>--------------|
				args.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{throw E(u8"[parser] expects expr");});
				//------------------------------------|
			}
			// else throw E(u8"[parser] expects '='");

			//|------------<INSERT>------------|
			node.args.push_back(std::move(args));
			//|--------------------------------|
			
			if (tkn = this->peek(); tkn == lexeme::COMMA)
			{
				this->next();
				//|--------//
				goto start;// <- back to the start
				//|--------//
			}
			// else throw E(u8"[parser] expects ','");
		}

		if (auto tkn {this->peek()}; tkn == lexeme::R_PAREN)
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if (auto tkn {this->peek()}; tkn == lexeme::COLON)
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next(); node.type = tkn->data;
		}
		else throw E(u8"[parser] expects <sym>");

		if (auto tkn {this->peek()}; tkn == lexeme::L_BRACE)
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|--------------<INSERT>--------------|
				node.body.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->decl_t()})
			{
				//|--------------<INSERT>--------------|
				node.body.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|--------------<INSERT>--------------|
				node.body.emplace_back(std::move(*out));
				//|------------------------------------|
				continue;
			}
			break;
		}

		if (auto tkn {this->peek()}; tkn == lexeme::R_BRACE)
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		return std::make_unique /*<wrap>*/
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

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_match() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$match node;

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_for() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$for node;

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_while() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$while node;

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_break() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$break node;

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_return() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$return node;

		return std::make_unique /*<wrap>*/
		<decltype(node)>(std::move(node));
	}

	auto stmt_continue() -> decltype(this->stmt_t())
	{
		this->next();

		lang::$continue node;

		return std::make_unique /*<wrap>*/
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
								{throw E(u8"[parser] expects expr");})};
								//------------------------------------|

								lang::$unary_l node
								{
									.lhs {((op::l(*tkn)))},
									.rhs {std::move(*rhs)},
								};

								return std::make_unique /*<wrap>*/
								<decltype(node)>(std::move(node));
							}
							// handle primary
							if (auto out {this->expr_call()})
							{
								return std::move(out);
							}
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
							{throw E(u8"[parser] expects expr");})};
							//------------------------------------|

							lang::$binary node
							{
								.lhs {std::move(*lhs)},
								.mhs {((op::i(*tkn)))},
								.rhs {std::move(*rhs)},
							};

							lhs = std::make_unique /*<wrap>*/
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

	auto expr_call() -> decltype(this->expr_t())
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case lexeme::CALL_M:
				case lexeme::CALL_S:
				case lexeme::CALL_U:
				case lexeme::CALL_N:
				{
					this->next();

					lang::$call node;

					node.type = op::r(*tkn);
					
					if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
					{
						this->next(); node.name = tkn->data;
					}
					else throw E(u8"[parser] expects <sym>");

					if (auto tkn {this->peek()}; tkn == lexeme::L_PAREN)
					{
						this->next(); // nothing to do
					}
					else throw E(u8"[parser] expects '('");

					start:
					if (auto ast {this->expr_t()})
					{
						//|------------<INSERT>------------|
						node.args.push_back(std::move(*ast));
						//|--------------------------------|

						if (tkn = this->peek(); tkn == lexeme::COMMA)
						{
							this->next();
							//|--------//
							goto start;// <- back to the start
							//|--------//
						}
						// else throw E(u8"[parser] expects ','");
					}

					if (auto tkn {this->peek()}; tkn == lexeme::R_PAREN)
					{
						this->next(); // nothing to do
					}
					else throw E(u8"[parser] expects ')'");

					return std::make_unique /*<wrap>*/
					<decltype(node)>(std::move(node));
				}
			}
		}
		return std::nullopt;
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
					{throw E(u8"[parser] expects expr");});
					//------------------------------------|

					if (this->peek() == lexeme::R_PAREN)
					{
						this->next();
					}
					else throw E(u8"[parser] expects ')'");

					return std::make_unique /*<wrap>*/
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

					node.name = tkn->data;

					return std::make_unique /*<wrap>*/
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

					node.data = tkn->data;
					node.type = data::BOOL;

					return std::make_unique /*<wrap>*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::CHAR:
				{
					this->next();

					lang::$literal node;

					node.data = tkn->data;
					node.type = data::CODE;

					return std::make_unique /*<wrap>*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::TEXT:
				{
					this->next();

					lang::$literal node;

					node.data = tkn->data;
					node.type = data::UTF8;

					return std::make_unique /*<wrap>*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::DEC:
				{
					this->next();

					lang::$literal node;

					node.data = tkn->data;
					node.type = data::F32;

					return std::make_unique /*<wrap>*/
					<decltype(node)>(std::move(node));
				}
				case lexeme::INT:
				case lexeme::BIN:
				case lexeme::OCT:
				case lexeme::HEX:
				{
					this->next();

					lang::$literal node;

					node.data = tkn->data;
					node.type = data::I32;

					return std::make_unique /*<wrap>*/
					<decltype(node)>(std::move(node));
				}
			}
		}
		return std::nullopt;
	}

	#undef E
};
