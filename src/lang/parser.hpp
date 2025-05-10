#pragma once

#include <memory>
#include <vector>
#include <cstdint>
#include <variant>
#include <functional>

#include "lang/lexer.hpp"

#include "./common/ast.hpp"
#include "./common/error.hpp"
#include "./common/token.hpp"

namespace
{
	[[nodiscard]]
	auto to_u(const lexeme type) -> op_u
	{
		switch (type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_u::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_u(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}

	[[nodiscard]]
	auto to_b(const lexeme type) -> op_b
	{
		switch (type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_b::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_b(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}

	[[nodiscard]]
	auto to_c(const lexeme type) -> op_c
	{
		switch (type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_c::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_c(macro)
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

	//--[data]--//
	uint16_t x; //
	uint16_t y; //
	//----------//

	//-----------------[buffer]-----------------//
	std::vector<decltype(lexer.pull())> buffer; //
	//------------------------------------------//

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

	[[nodiscard]]
	auto next()
	{
		auto result {this->lexer.pull()};

		// step 1. accumulate buffer
		this->buffer.push_back(result);

		// step 2. update x & y position
		std::visit([&](auto&& arg)
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, error>)
			{
				this->x = arg.x;
				this->y = arg.y;
			}
			if constexpr (std::is_same_v<T, token<B>>)
			{
				this->x = arg.x;
				this->y = arg.y;
			}
		},
		result); // <- we put result here
	}

	[[nodiscard]]
	auto peek() -> token<B>*
	{
		return std::get_if<token<B>>(&this->buffer.back());
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

	[[nodiscard]]
	auto pull() -> std::variant<program, error>
	{
		program ast;

		try // start parser..!
		{
			while (auto decl {this->parse_decl()})
			{
				ast.body.push_back(std::move(decl));
			}
		}
		catch (error& error)
		{
			return error;
		}
		return ast;
	}

	[[nodiscard]]
	auto print()
	{
		// TODO
	}

private:

	//|------------|
	//| statements |
	//|------------|

	[[nodiscard]]
	auto parse_stmt() -> stmt
	{

	}

	[[nodiscard]]
	auto stmt_for() -> stmt
	{

	}

	[[nodiscard]]
	auto stmt_while() -> stmt
	{

	}

	[[nodiscard]]
	auto stmt_if() -> stmt
	{

	}

	[[nodiscard]]
	auto stmt_match() -> stmt
	{

	}

	//|-------------|
	//| expressions |
	//|-------------|

	[[nodiscard]]
	auto parse_expr() -> expr
	{
		struct meta_t
		{
			enum : uint8_t
			{
				LHS,
				RHS,
			}
			side; // associvity

			enum : uint8_t
			{
				NIL = 0,
				_01 = 1,
				_02 = 2,
				_03 = 3,
				_04 = 4,
				_05 = 5,
				_06 = 6,
				_07 = 7,
				_08 = 8,
				_09 = 9,
				_10 = 10,
				_11 = 11,
				MAX = 12,
			}
			priority; // precedence

		public:

			[[nodiscard]]
			operator bool() const
			{
				return this->priority != NIL;
			}

			[[nodiscard]]
			auto operator<=>(const meta_t& rhs) const
			{
				return this->priority <=> rhs.priority;
			}

			[[nodiscard]]
			auto operator==(const meta_t& rhs) const
			{
				return this->priority == rhs.priority;
			}

			[[nodiscard]]
			auto upgrade() const -> uint8_t
			{
				return (this->priority < MAX)
					   ?
					   (this->priority + 0x1)
					   :
					   (this->priority + 0x0);
			}
		};

		const auto node
		{
			[&]
			{
				if (auto tkn {this->peek()})
				{
					switch (tkn->type)
					{
						case lexeme::AD_AT:
						case lexeme::AD_OF:
						case lexeme::L_NOT:
						case lexeme::B_NOT:
						{
							// consume
							this->next();
							// flush-out
							return std::make_unique<lang::_unary>
							({
								to_u(tkn->type), std::move(this->parse_expr())
							});
						}
						default: // primary
						{
							expr ptr {nullptr};

							if ((ptr = this->expr_call())) { return ptr; }
							if ((ptr = this->expr_group())) { return ptr; }
							if ((ptr = this->expr_symbol())) { return ptr; }
							if ((ptr = this->expr_literal())) { return ptr; }

							throw E(u8"[parser.hpp] invalid primary expr");
						}
					}
				}
				throw E(u8"[parser.hpp] EOF whilst parsing prefix");
			}
		};

		const auto data
		{
			[](token<B>& tkn) -> meta_t
			{
				switch (tkn.type)
				{
					case lexeme::ASSIGN_STD:
					case lexeme::ASSIGN_ADD:
					case lexeme::ASSIGN_SUB:
					case lexeme::ASSIGN_MUL:
					case lexeme::ASSIGN_DIV:
					case lexeme::ASSIGN_MOD:
					case lexeme::ASSIGN_POW:
					{
						return { meta_t::RHS, meta_t::_01 };
					}
					case lexeme::COALESCE:
					{
						return { meta_t::LHS, meta_t::_02 };
					}
					case lexeme::L_OR:
					case lexeme::B_OR:
					{
						return { meta_t::LHS, meta_t::_03 };
					}
					case lexeme::L_AND:
					case lexeme::B_AND:
					{
						return { meta_t::LHS, meta_t::_04 };
					}
					case lexeme::EQ:
					case lexeme::NE:
					{
						return { meta_t::LHS, meta_t::_05 };
					}
					case lexeme::GT:
					case lexeme::LT:
					case lexeme::GTE:
					case lexeme::LTE:
					{
						return { meta_t::LHS, meta_t::_06 };
					}
					case lexeme::SHL:
					case lexeme::SHR:
					{
						return { meta_t::LHS, meta_t::_07 };
					}
					case lexeme::ADD:
					case lexeme::SUB:
					{
						return { meta_t::LHS, meta_t::_08 };
					}
					case lexeme::MUL:
					case lexeme::DIV:
					case lexeme::MOD:
					{
						return { meta_t::LHS, meta_t::_09 };
					}
					case lexeme::POW:
					{
						return { meta_t::RHS, meta_t::_10 };
					}
					default: // well...
					{
						return { meta_t::LHS, meta_t::NIL };
					}
				}
			}
		};

		// pratt parser impl
		std::function<expr(uint8_t)> impl
		{
			[&](const uint8_t min) -> expr
			{
				//-----------------//
				auto lhs {node()}; // <- create node
				//-----------------//

				while (true)
				{
					const auto tkn {this->peek()};
					if (!tkn) { break; }
					const auto meta {data(*tkn)};
					if (!meta) { break; }

					if (meta < min)
					{
						//------------//
						this->next(); // <- consume op
						//------------//

						auto rhs {impl
						(
							meta.side
							!=
							meta_t::LHS
							?
							meta.priority
							:
							meta.upgrade()
						)};
						
						//|---------------------------------|
						//| grow AST by calling recursively |
						//|---------------------------------|
						lhs = std::make_unique<lang::_binary>
						({
							std::move(lhs),
							to_u(tkn->type),
							std::move(rhs),
						});
					}
					else
					{
						break;
					}
				}
				return lhs;
			}
		};
		return impl(meta_t::NIL);
	}

	[[nodiscard]]
	auto expr_call() -> expr
	{
		return nullptr;
	}

	[[nodiscard]]
	auto expr_group() -> expr
	{
		return nullptr;
	}

	[[nodiscard]]
	auto expr_symbol() -> expr
	{
		return nullptr;
	}

	[[nodiscard]]
	auto expr_literal() -> expr
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case lexeme::TRUE:
				case lexeme::FALSE:
				{
					//------------//
					this->next(); //
					//------------//
					return std::make_unique<lang::_literal>({ tkn->data, data::BOOL });
				}
				case lexeme::CHAR:
				{
					//------------//
					this->next(); //
					//------------//
					return std::make_unique<lang::_literal>({ tkn->data, data::CODE });
				}
				case lexeme::TEXT:
				{
					//------------//
					this->next(); //
					//------------//
					return std::make_unique<lang::_literal>({ tkn->data, data::UTF8 });
				}
				case lexeme::DEC:
				{
					//------------//
					this->next(); //
					//------------//
					return std::make_unique<lang::_literal>({ tkn->data, data::F32 });
				}
				case lexeme::INT:
				case lexeme::BIN:
				case lexeme::OCT:
				case lexeme::HEX:
				{
					//------------//
					this->next(); // TODO: calculate minimum storage
					//------------//
					return std::make_unique<lang::_literal>({tkn->data, data::I32 });
				}
			}
		}
		return nullptr;
	}

	//|--------------|
	//| declarations |
	//|--------------|

	[[nodiscard]]
	auto parse_decl() -> decl
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

	[[nodiscard]]
	auto decl_var(const bool is_const) -> decl
	{
		this->next();
		
		lang::_var ast;
		// update metadata
		ast.is_const = is_const;

		// let name: T = init?;
		// let! name: T = init;

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			ast.name = tkn->data.to_utf8(); this->next();
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::COLON)
		{
			/* syntax, nothing to do here...*/ this->next();
		}
		else { throw E(u8"[parser.hpp] expects lexeme::COLON"); }

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			ast.type = tkn->data.to_utf8(); this->next();
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }

		return std::make_unique<decltype(ast)>(std::move(ast));
	}

	[[nodiscard]]
	auto decl_fun(const bool is_pure) -> decl
	{
		this->next();

		lang::_fun ast;
		// update metadata
		ast.is_pure = is_pure;

		// fun name(param?) block
		// fun! name(param?) block

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			ast.name = tkn->data.to_utf8(); this->next();
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }
	
		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::L_PAREN)
		{
			/* syntax, nothing to do here...*/ this->next();
		}
		else { throw E(u8"[parser.hpp] expects lexeme::L_PAREN"); }

		// TODO: args

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::R_PAREN)
		{
			/* syntax, nothing to do here...*/ this->next();
		}
		else { throw E(u8"[parser.hpp] expects lexeme::R_PAREN"); }

		return std::make_unique<decltype(ast)>(std::move(ast));
	}

	#undef E
};
