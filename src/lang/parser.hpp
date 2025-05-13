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

	auto peek() -> std::optional<token<B>>
	{
		if (const auto ptr {std::get_if<token<B>>(&this->buffer.back())})
		{ return *ptr; /* fuck you GCC */ } else { return std::nullopt; }
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

	auto pull() -> std::variant<program, error>
	{
		program ast;

		try // start parser..!
		{
			while (auto decl {this->$decl()})
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

	//|-------------|
	//| expressions |
	//|-------------|

	auto $expr() -> expr
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
				NIL = 0x0,
				_01 = 0x1,
				_02 = 0x2,
				_03 = 0x3,
				_04 = 0x4,
				_05 = 0x5,
				_06 = 0x6,
				_07 = 0x7,
				_08 = 0x8,
				_09 = 0x9,
				_10 = 0xA,
				_11 = 0xB,
				MAX = 0xC,
			}
			priority; // precedence

		public:

			operator bool() const
			{
				return this->priority != NIL;
			}

			auto operator==(const meta_t& rhs) const
			{
				return this->priority == rhs.priority;
			}

			auto operator<=>(const meta_t& rhs) const
			{
				return this->priority <=> rhs.priority;
			}

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
			[&] -> expr
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
							//------------//
							this->next(); // <- consume op
							//------------//

							lang::_unary node;

							node.op = to_u(tkn->type);
							node.rhs = this->$expr();

							return std::make_unique<decltype(node)>(std::move(node));
						}
						default: // primary
						{
							expr ptr {nullptr};

							if ((ptr = std::move(this->expr_call()))) { return ptr; }
							if ((ptr = std::move(this->expr_group()))) { return ptr; }
							if ((ptr = std::move(this->expr_symbol()))) { return ptr; }
							if ((ptr = std::move(this->expr_literal()))) { return ptr; }

							throw E(u8"[parser.hpp] invalid primary expr");
						}
					}
				}
				throw E(u8"[parser.hpp] EOF whilst parsing prefix");
			}
		};

		const auto data
		{
			[&](const token<B>& tkn) -> meta_t
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
						return { meta_t::RHS, meta_t::MAX };
					}
					case lexeme::COALESCE:
					{
						return { meta_t::LHS, meta_t::_11 };
					}
					case lexeme::L_OR:
					case lexeme::B_OR:
					{
						return { meta_t::LHS, meta_t::_10 };
					}
					case lexeme::L_AND:
					case lexeme::B_AND:
					{
						return { meta_t::LHS, meta_t::_09 };
					}
					case lexeme::EQ:
					case lexeme::NE:
					{
						return { meta_t::LHS, meta_t::_08 };
					}
					case lexeme::GT:
					case lexeme::LT:
					case lexeme::GTE:
					case lexeme::LTE:
					{
						return { meta_t::LHS, meta_t::_07 };
					}
					case lexeme::SHL:
					case lexeme::SHR:
					{
						return { meta_t::LHS, meta_t::_06 };
					}
					case lexeme::ADD:
					case lexeme::SUB:
					{
						return { meta_t::LHS, meta_t::_05 };
					}
					case lexeme::MUL:
					case lexeme::DIV:
					case lexeme::MOD:
					{
						return { meta_t::LHS, meta_t::_04 };
					}
					case lexeme::POW:
					{
						return { meta_t::LHS, meta_t::_03 };
					}
					case lexeme::AD_AT:
					case lexeme::AD_OF:
					case lexeme::L_NOT:
					case lexeme::B_NOT:
					{
						return { meta_t::RHS, meta_t::_02 };
					}
					case lexeme::CALL_M:
					case lexeme::CALL_S:
					case lexeme::CALL_U:
					case lexeme::CALL_N:
					// group
					case lexeme::L_PAREN:
					{
						return { meta_t::LHS, meta_t::_01 };
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

					if (!(meta < min))
					{
						break;
					}
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
					lang::_binary node;

					node.lhs = std::move(lhs);
					node.op = to_b(tkn->type);
					node.rhs = std::move(rhs);
					
					lhs = std::make_unique<decltype(node)>(std::move(node));
				}
				return lhs;
			}
		};
		return impl(meta_t::NIL);
	}

	auto expr_call() -> expr
	{
		return nullptr;
	}

	auto expr_group() -> expr
	{
		return nullptr;
	}

	auto expr_symbol() -> expr
	{
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
					//------------//
					this->next(); // <- consume op
					//------------//
					lang::_literal node;

					node.data = tkn->data;
					node.type = data::BOOL;

					return std::make_unique<decltype(node)>(std::move(node));
				}
				case lexeme::CHAR:
				{
					//------------//
					this->next(); // <- consume op
					//------------//
					lang::_literal node;

					node.data = tkn->data;
					node.type = data::CODE;

					return std::make_unique<decltype(node)>(std::move(node));
				}
				case lexeme::TEXT:
				{
					//------------//
					this->next(); // <- consume op
					//------------//
					lang::_literal node;
					
					node.data = tkn->data;
					node.type = data::UTF8;

					return std::make_unique<decltype(node)>(std::move(node));
				}
				case lexeme::DEC:
				{
					//------------//
					this->next(); // <- consume op
					//------------//
					lang::_literal node;
					
					node.data = tkn->data;
					node.type = data::F32;

					return std::make_unique<decltype(node)>(std::move(node));
				}
				case lexeme::INT:
				case lexeme::BIN:
				case lexeme::OCT:
				case lexeme::HEX:
				{
					//------------//
					this->next(); // TODO: calculate minimum storage
					//------------//
					lang::_literal node;
					
					node.data = tkn->data;
					node.type = data::I32;

					return std::make_unique<decltype(node)>(std::move(node));
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

		//-------------------------------------|
		// let ::= "let" symbol ":" T = expr?; |
		//-------------------------------------|

		//--------------------------------------|
		// let! ::= "let!" symbol ":" T = expr; |
		//--------------------------------------|

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			this->next(); ast.name = tkn->data;
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::COLON)
		{
			this->next(); // nothing to do here
		}
		else { throw E(u8"[parser.hpp] expects lexeme::COLON"); }

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			this->next(); ast.type = tkn->data;
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }

		if (!ast.is_const)
		{
			if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::ASSIGN)
			{
				this->next(); ast.init = this->$expr();
			}
			// else { throw E(u8"[parser.hpp] must init const variable"); }

			if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::S_COLON)
			{
				this->next(); // nothing to do here
			}
			else { throw E(u8"[parser.hpp] expects lexeme::S_COLON"); }
		}
		else // let! name
		{
			if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::ASSIGN)
			{
				this->next(); ast.init = this->$expr();
			}
			else { throw E(u8"[parser.hpp] must init const variable"); }

			if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::S_COLON)
			{
				this->next(); // nothing to do here
			}
			else { throw E(u8"[parser.hpp] expects lexeme::S_COLON"); }
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

		//-------------------------------------------------------|
		// fun ::= "fun" name "(" param? ")" ":" T "{" stmt* "}" |
		//-------------------------------------------------------|

		//---------------------------------------------------------|
		// fun! ::= "fun!" name "(" param? ")" ":" T "{" stmt* "}" | 
		//---------------------------------------------------------|

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			this->next(); ast.name = tkn->data;
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }
	
		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::L_PAREN)
		{
			this->next(); // nothing to do here
		}
		else { throw E(u8"[parser.hpp] expects lexeme::L_PAREN"); }

		args:
		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			this->next();
			
			lang::_var arg;
			// update metadata
			arg.name = tkn->data;

			if (tkn = this->peek(); tkn && tkn->type == lexeme::COLON)
			{
				this->next(); // nothing to do here
			}
			else { throw E(u8"[parser.hpp] expects lexeme::COLON"); }

			if (tkn = this->peek(); tkn && tkn->type == lexeme::SYMBOL)
			{
				this->next(); arg.name = tkn->data;
			}
			else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }

			if (tkn = this->peek(); tkn && tkn->type == lexeme::ASSIGN)
			{
				this->next(); arg.init = this->$expr();
			}
			// else { throw E(u8"[parser.hpp] expects lexeme::ASSIGN"); }

			//-------------<INSERT>-------------//
			ast.args.push_back(std::move(arg)); //
			//----------------------------------//
			
			if (tkn = this->peek(); tkn && tkn->type == lexeme::COMMA)
			{
				this->next(); goto args; // <- *wink*
			}
			// else { throw E(u8"[parser.hpp] expects lexeme::COMMA"); }
		}

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::R_PAREN)
		{
			this->next(); // nothing to do here
		}
		else { throw E(u8"[parser.hpp] expects lexeme::R_PAREN"); }

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::COLON)
		{
			this->next(); // nothing to do here
		}
		else { throw E(u8"[parser.hpp] expects lexeme::COLON"); }

		if (auto tkn {this->peek()}; tkn && tkn->type == lexeme::SYMBOL)
		{
			this->next(); ast.type = tkn->data;
		}
		else { throw E(u8"[parser.hpp] expects lexeme::SYMBOL"); }

		if (!ast.is_pure)
		{

		}
		else // fun! name
		{

		}
		return std::make_unique<decltype(ast)>(std::move(ast));
	}

	#undef E
};
