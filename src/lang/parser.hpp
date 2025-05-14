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
	auto to_l(const token<B>& tkn) -> op_l
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
	auto to_i(const token<B>& tkn) -> op_i
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
	auto to_r(const token<B>& tkn) -> op_r
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

	//--<data>--//
	uint16_t x; //
	uint16_t y; //
	//----------//

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

	typedef std::optional<token<B>> maybe;

	//-----------------[buffer]-----------------//
	std::vector<decltype(lexer.pull())> buffer; //
	//------------------------------------------//

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

			if constexpr (std::is_same_v<T, eof>)
			{
				return std::nullopt;
			}
			else if constexpr (std::is_same_v<T, error>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return std::nullopt;
			}
			else if constexpr (std::is_same_v<T, token<B>>)
			{
				this->x = arg.x;
				this->y = arg.y;

				#ifndef NDEBUG //------------|
				std::cout << arg << std::endl;
				#endif //--------------------|

				return /**/ arg /**/;
			}
		},
		this->buffer.back());
	}

public:

	//|---------------|
	//| the rule of 0 |
	//|---------------|

	parser(decltype(lexer) lexer) : lexer {lexer}
	{
		this->next(); // on creation, pull a token
	}

	//|-----------------|
	//| member function |
	//|-----------------|

	auto pull() -> std::optional<program>
	{
		try
		{
			program program;
			
			while (auto decl {this->$decl()})
			{
				//|---------------<INSERT>---------------//
				program.body.push_back(std::move(decl)); //
				//|--------------------------------------//
			}
			return program;
		}
		catch (const error& error)
		{
			#ifndef NDEBUG //-------------|
			std::cout << error << std::endl;
			#endif //---------------------|

			return std::nullopt;
		}
	}

	auto print()
	{
		// TODO
	}

private:

	//|------------|
	//| statements |
	//|------------|

	auto $stmt() -> stmt
	{
		if (const auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case lexeme::LET:
				{
					return this->stmt_for();
				}
				case lexeme::LET$:
				{
					return this->stmt_while();
				}
				case lexeme::FUN:
				{
					return this->stmt_if();
				}
				case lexeme::FUN$:
				{
					return this->stmt_match();
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
		return nullptr;
	}

	auto stmt_for() -> stmt
	{
		return nullptr;
	}

	auto stmt_while() -> stmt
	{
		return nullptr;
	}

	auto stmt_if() -> stmt
	{
		return nullptr;
	}

	auto stmt_match() -> stmt
	{
		return nullptr;
	}

	auto stmt_break() -> stmt
	{
		return nullptr;
	}

	auto stmt_return() -> stmt
	{
		return nullptr;
	}

	auto stmt_continue() -> stmt
	{
		return nullptr;
	}

	//|-------------|
	//| expressions |
	//|-------------|

	auto $expr() -> expr
	{
		static const auto prefix
		{
			[](const token<B>& tkn) -> std::array<uint8_t, 2>
			{
				switch (tkn.type)
				{
					case lexeme::AD_AT:
					case lexeme::AD_OF:
					case lexeme::L_NOT:
					case lexeme::B_NOT:
					{
						return {69, 74};
					}
					default:
					{
						assert(!!!"error");
						std::unreachable();
					}
				}
			}
		};
		
		static const auto infix
		{
			[](const token<B>& tkn) -> std::array<uint8_t, 2>
			{
				switch (tkn.type)
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
		};

		static const auto postfix
		{
			[](const token<B>& tkn) -> std::array<uint8_t, 2>
			{
				switch (tkn.type)
				{
					case lexeme::CALL_M:
					case lexeme::CALL_S:
					case lexeme::CALL_U:
					case lexeme::CALL_N:
					{
						return {69, 74};
					}
					default:
					{
						assert(!!!"error");
						std::unreachable();
					}
				}
			}
		};

		// pratt parser impl
		std::function<expr(uint8_t)> impl
		{
			[&](const uint8_t mbp) -> expr
			{
				auto lhs
				{
					[&] -> expr
					{
						if (auto tkn {this->peek()})
						{
							// handle prefix
							if (op::is_l(*tkn))
							{
								this->next();
								
								lang::_unary_l node;

								node.lhs = op::to_l(*tkn);
								node.rhs = impl(UINT8_MAX);

								return std::make_unique // forge
								<decltype(node)>(std::move(node));
							}
							// handle primary
							expr ptr {nullptr};

							if (ptr = std::move(this->expr_call()))
							{
								return std::move(ptr);
							}
							if (ptr = std::move(this->expr_group()))
							{
								return std::move(ptr);
							}
							if (ptr = std::move(this->expr_symbol()))
							{
								return std::move(ptr);
							}
							if (ptr = std::move(this->expr_literal()))
							{
								return std::move(ptr);
							}
							throw E(u8"[parser.hpp] invalid prefix");
						}
						else
						{
							throw E(u8"[parser.hpp] syntax error");
						}
					}
					()
				};

				while (auto tkn {this->peek()})
				{
					// handle postfix
					if (op::is_r(*tkn))
					{
						auto [lbp, rbp]
						{
							postfix(*tkn)
						};
						if (lbp < mbp)
						{
							break;
						}
						this->next();
						
						lang::_unary_r node;

						node.lhs = std::move(lhs);
						node.rhs = op::to_r(*tkn);

						lhs = std::make_unique // forge
						<decltype(node)>(std::move(node));
						continue;
					}

					// handle infix
					if (op::is_i(*tkn))
					{
						auto [lbp, rbp]
						{
							infix(*tkn)
						};
						if (lbp < mbp)
						{
							break;
						}
						this->next();
						
						lang::_binary node;

						auto rhs {impl(rbp)};

						node.lhs = std::move(lhs);
						node.mhs = op::to_i(*tkn);
						node.rhs = std::move(rhs);

						lhs = std::make_unique // forge
						<decltype(node)>(std::move(node));
						continue;
					}
					break;
				}
				return lhs;
			}
		};
		return impl(0);
	}

	auto expr_call() -> expr
	{
		return nullptr;
	}

	auto expr_group() -> expr
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case lexeme::L_PAREN:
				{
					this->next(); // <- consume '('
					
					lang::_group node;
					node.body = this->$expr();

					if (this->peek() == lexeme::R_PAREN)
					{
						this->next(); // <- consume ')'
					}
					else throw E(u8"[parser.hpp] expects ')'");

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
			}
		}
		return nullptr;
	}

	auto expr_symbol() -> expr
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case lexeme::SYMBOL:
				{
					this->next();

					lang::_symbol node;
					node.name = tkn->data;

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
			}
		}
		return nullptr;
	}

	auto expr_literal() -> expr
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case lexeme::TRUE:
				case lexeme::FALSE:
				{
					this->next();
					
					lang::_literal node;
					node.data = tkn->data;
					node.type = data::BOOL;

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
				case lexeme::CHAR:
				{
					this->next();

					lang::_literal node;
					node.data = tkn->data;
					node.type = data::CODE;

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
				case lexeme::TEXT:
				{
					this->next();

					lang::_literal node;
					node.data = tkn->data;
					node.type = data::UTF8;

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
				case lexeme::DEC:
				{
					this->next();

					lang::_literal node;
					node.data = tkn->data;
					node.type = data::F32;

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
				case lexeme::INT:
				case lexeme::BIN:
				case lexeme::OCT:
				case lexeme::HEX:
				{
					this->next();

					lang::_literal node;
					node.data = tkn->data;
					node.type = data::I32;

					return std::make_unique // forge
					<decltype(node)>(std::move(node));
				}
			}
		}
		return nullptr;
	}

	//|--------------|
	//| declarations |
	//|--------------|

	auto $decl() -> decl
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
		return nullptr;
	}

	auto decl_var(const bool is_const) -> decl
	{
		this->next();
		
		lang::_var ast;
		// update metadata
		ast.is_const = is_const;

		//|-------------------------------------|
		//| let ::= "let" symbol ":" T = expr?; |
		//|-------------------------------------|

		//|--------------------------------------|
		//| let! ::= "let!" symbol ":" T = expr; |
		//|--------------------------------------|

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next(); ast.name = tkn->data;
		}
		else throw E(u8"[parser.hpp] expects <sym>");

		if (auto tkn {this->peek()}; tkn == lexeme::COLON)
		{
			this->next(); // nothing to do here
		}
		else throw E(u8"[parser.hpp] expects ':'");

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next(); ast.type = tkn->data;
		}
		else throw E(u8"[parser.hpp] expects <sym>");

		if (!ast.is_const)
		{
			if (auto tkn {this->peek()}; tkn == lexeme::ASSIGN)
			{
				this->next(); ast.init = this->$expr();
			}
			// else { throw E(u8"[parser.hpp] must init const variable"); }

			if (auto tkn {this->peek()}; tkn == lexeme::S_COLON)
			{
				this->next(); // nothing to do here
			}
			else throw E(u8"[parser.hpp] expects ';'");
		}
		else // let! name
		{
			if (auto tkn {this->peek()}; tkn == lexeme::ASSIGN)
			{
				this->next(); ast.init = this->$expr();
			}
			else throw E(u8"[parser.hpp] must init const var");

			if (auto tkn {this->peek()}; tkn == lexeme::S_COLON)
			{
				this->next(); // nothing to do here
			}
			else throw E(u8"[parser.hpp] expects ':'");
		}
		return std::make_unique<decltype(ast)>(std::move(ast));
	}

	auto decl_fun(const bool is_pure) -> decl
	{
		this->next();

		lang::_fun ast;
		// update metadata
		ast.is_pure = is_pure;

		//|---------------------------------------------------|
		//| param ::= symbol ":" T ( "=" expr )? ("," param)? |
		//|---------------------------------------------------|

		//|-------------------------------------------------------|
		//| fun ::= "fun" name "(" param? ")" ":" T "{" stmt* "}" |
		//|-------------------------------------------------------|

		//|---------------------------------------------------------|
		//| fun! ::= "fun!" name "(" param? ")" ":" T "{" stmt* "}" | 
		//|---------------------------------------------------------|

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next(); ast.name = tkn->data;
		}
		else throw E(u8"[parser.hpp] expects <sym>");
	
		if (auto tkn {this->peek()}; tkn == lexeme::L_PAREN)
		{
			this->next(); // nothing to do here
		}
		else throw E(u8"[parser.hpp] expects '('");

		args:
		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next();
			
			lang::_var arg;
			// update metadata
			arg.name = tkn->data;

			if (tkn = this->peek(); tkn == lexeme::COLON)
			{
				this->next(); // nothing to do here
			}
			else throw E(u8"[parser.hpp] expects ':'");

			if (tkn = this->peek(); tkn == lexeme::SYMBOL)
			{
				this->next(); arg.type = tkn->data;
			}
			else throw E(u8"[parser.hpp] expects <sym>");

			if (tkn = this->peek(); tkn == lexeme::ASSIGN)
			{
				this->next(); arg.init = this->$expr();
			}
			// else throw E(u8"[parser.hpp] expects '='");

			//|------------<INSERT>-------------//
			ast.args.push_back(std::move(arg)); //
			//|---------------------------------//
			
			if (tkn = this->peek(); tkn == lexeme::COMMA)
			{
				this->next(); goto args; // <- *wink*
			}
			// else throw E(u8"[parser.hpp] expects ','");
		}

		if (auto tkn {this->peek()}; tkn == lexeme::R_PAREN)
		{
			this->next(); // nothing to do here
		}
		else throw E(u8"[parser.hpp] expects ')'");

		if (auto tkn {this->peek()}; tkn == lexeme::COLON)
		{
			this->next(); // nothing to do here
		}
		else throw E(u8"[parser.hpp] expects ':'");

		if (auto tkn {this->peek()}; tkn == lexeme::SYMBOL)
		{
			this->next(); ast.type = tkn->data;
		}
		else throw E(u8"[parser.hpp] expects <sym>");

		if (auto tkn {this->peek()}; tkn == lexeme::L_BRACE)
		{
			this->next(); // nothing to do here
		}
		else throw E(u8"[parser.hpp] expects '{'");

		while (auto stmt {this->$stmt()})
		{
			//|-------------<INSERT>-------------//
			ast.body.push_back(std::move(stmt)); //
			//|----------------------------------//
		}

		if (auto tkn {this->peek()}; tkn == lexeme::R_BRACE)
		{
			this->next(); // nothing to do here
		}
		else throw E(u8"[parser.hpp] expects '}'");

		return std::make_unique<decltype(ast)>(std::move(ast));
	}

	#undef E
};
