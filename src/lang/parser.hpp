#pragma once

#include <array>
#include <memory>
#include <cassert>
#include <cstdint>
#include <variant>
#include <utility>
#include <optional>
#include <iostream>
#include <type_traits>

#include "./lexer.hpp"

#include "lang/common/ast.hpp"
#include "lang/common/eof.hpp"
#include "lang/common/token.hpp"
#include "lang/common/error.hpp"

template
<
	typename A,
	typename B
>
class parser
{
	lexer<A, B>* lexer;

	#define E(value) error<A, B> \
	{                            \
	    this->x,                 \
	    this->y,                 \
	    *this,                   \
	    value,                   \
	}                            \
	
	uint16_t x {0};
	uint16_t y {0};

	AST<A, B> exe;

	//|---------<buffer>---------|
	decltype(lexer->pull()) buffer;
	//|--------------------------|

	//|---------------<maybe>--------------|
	typedef std::optional<token<A, B>> maybe;
	//|------------------------------------|

	inline constexpr auto peek() -> maybe
	{
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (std::is_same_v<T, token<A, B>>)
			{
				return arg;
			}
			if constexpr (std::is_same_v<T, error<A, B>>)
			{
				throw arg;
			}
			return std::nullopt;
		},
		this->buffer);
	}

	inline constexpr auto next() -> maybe
	{
		// step 1. update buffer
		this->buffer = this->lexer->pull();

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> maybe
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (!std::is_same_v<T, eof>)
			{
				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|
				this->x = arg.x;
				this->y = arg.y;
			}
			return this->peek();
		},
		this->buffer);
	}

	inline constexpr auto peek(const atom type) -> bool
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
				throw arg; // rawr!
			}
			return false;
		},
		this->buffer);
	}

	inline constexpr auto next(const atom type) -> bool
	{
		// step 1. update buffer
		this->buffer = this->lexer->pull();

		// step 2. update x & y position
		return std::visit([&](auto&& arg) -> bool
		{
			typedef std::decay_t<decltype(arg)> T;

			if constexpr (!std::is_same_v<T, eof>)
			{
				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|
				this->x = arg.x;
				this->y = arg.y;
			}
			return this->peek(type);
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
				#ifndef NDEBUG //-------|
				std::cout << arg << '\n';
				#endif //---------------|
				this->x = arg.x;
				this->y = arg.y;
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

	inline constexpr auto pull() -> AST<A, B>
	{
		while (this->peek())
		{
			try
			{
				if (auto out {this->_decl()})
				{
					std::visit([&](auto&& ptr)
					{
						this->exe.body.emplace_back(std::forward<decltype(ptr)>(ptr));
					},
					std::move(*out)); // unwrap
					continue;
				}
				if (auto out {this->_stmt()})
				{
					std::visit([&](auto&& ptr)
					{
						this->exe.body.emplace_back(std::forward<decltype(ptr)>(ptr));
					},
					std::move(*out)); // unwrap
					continue;
				}
				throw E(u8"unknown decl/stmt");
			}
			catch (error<A, B>& out)
			{
				// recovery
				this->sync();
				// diagnostics
				this->exe.lint
				.emplace_back(out);
			}
		}
		return std::move(this->exe);
	}

private:

	inline constexpr auto sync()
	{
		start:
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::S_COLON:
				case atom::R_BRACE:
				case atom::R_BRACK:
				case atom::R_PAREN:
				{
					this->next();
					goto close;
				}
				default:
				{
					this->next();
					goto start;
				}
			}
		}
		close:
		return;
	}

	inline constexpr auto _decl() -> std::optional<decl>
	{
		try
		{
			if (auto tkn {this->peek()})
			{
				switch (tkn->type)
				{
					case atom::LET:
					{
						return this->decl_var(false);
					}
					case atom::LET_:
					{
						return this->decl_var(true);
					}
					case atom::FUN:
					{
						return this->decl_fun(false);
					}
					case atom::FUN_:
					{
						return this->decl_fun(true);
					}
					case atom::MODEL:
					{
						return this->decl_model();
					}
					case atom::TRAIT:
					{
						return this->decl_trait();
					}
				}
			}
			return std::nullopt;
		}
		catch (error<A, B>& out)
		{
			// recovery
			this->sync();
			// diagnostics
			this->exe.lint
			.emplace_back(out);
		}
		return this->_decl();
	}

	inline constexpr auto decl_var(const bool only) -> decl
	{
		auto ast {std::make_unique<var_decl>()};
		
		ast->x = this->x;
		ast->y = this->y;

		this->next();

		ast->only = only;

		if (this->peek(atom::SYMBOL))
		{
			ast->name = // move
			this->peek()->data;

			this->next();
		}
		else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"expects ':'");

		if (this->peek(atom::SYMBOL))
		{
			ast->type = // move
			this->peek()->data;

			this->next();
		}
		else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::ASSIGN))
		{
			this->next();

			if (auto out {this->_expr()})
			{
				ast->init = std::move(out);
			}
			else throw E(u8"invalid expr");
		}
		// else throw u8"must init";

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"expects ';'");

		return ast;
	}

	inline constexpr auto decl_fun(const bool pure) -> decl
	{
		auto ast {std::make_unique<fun_decl>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		ast->pure = pure;

		if (this->peek(atom::SYMBOL))
		{
			ast->name = // move
			this->peek()->data;

			this->next();
		}
		else throw E(u8"expects 'ƒ'");
	
		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects '('");

		_start_:
		// fresh construct
		fun_decl::data data;

		if (this->peek(atom::SYMBOL))
		{
			data.name = // move
			this->peek()->data;

			this->next();

			if (this->peek(atom::COLON))
			{
				this->next();
			}
			else throw E(u8"expects ':'");

			if (this->peek(atom::SYMBOL))
			{
				data.type = // move
				this->peek()->data;

				this->next();
			}
			else throw E(u8"expects 'ƒ'");

			ast->args.emplace_back(data);
			
			if (this->peek(atom::COMMA))
			{
				this->next();
				goto _start_;
			}
		}
		// else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects ')'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"expects ':'");

		if (this->peek(atom::SYMBOL))
		{
			ast->type = // move
			this->peek()->data;

			this->next();
		}
		else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '{'");

		while (true)
		{
			if (auto out {this->_decl()})
			{
				std::visit([&](auto&& ptr)
				{
					ast->body.emplace_back(std::forward<decltype(ptr)>(ptr));
				},
				std::move(*out)); // unwrap
				continue;
			}
			if (auto out {this->_stmt()})
			{
				std::visit([&](auto&& ptr)
				{
					ast->body.emplace_back(std::forward<decltype(ptr)>(ptr));
				},
				std::move(*out)); // unwrap
				continue;
			}
			if (auto out {this->_expr()})
			{
				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"expects ';'");

				std::visit([&](auto&& ptr)
				{
					ast->body.emplace_back(std::forward<decltype(ptr)>(ptr));
				},
				std::move(*out)); // unwrap
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '}'");

		return ast;
	}

	inline constexpr auto decl_model() -> decl
	{
		auto ast {std::make_unique<model_decl>()};
		
		ast->x = this->x;
		ast->y = this->y;

		this->next();

		if (this->peek(atom::SYMBOL))
		{
			ast->name = // move
			this->peek()->data;

			this->next();
		}
		else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '{'");

		_start_:
		// fresh construct
		model_decl::data data;

		if (this->peek(atom::SYMBOL))
		{
			data.name = // move
			this->peek()->data;

			this->next();

			if (this->peek(atom::COLON))
			{
				this->next();
			}
			else throw E(u8"expects ':'");

			if (this->peek(atom::SYMBOL))
			{
				data.type = // move
				this->peek()->data;

				this->next();
			}
			else throw E(u8"expects 'ƒ'");

			if (this->peek(atom::S_COLON))
			{
				this->next();
			}
			else throw E(u8"expects ';'");

			//|--------------<insert>--------------|
			ast->body.emplace_back(std::move(data));
			//|------------------------------------|
			
			goto _start_;
		}
		// else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '}'");

		return ast;
	}

	inline constexpr auto decl_trait() -> decl
	{
		auto ast {std::make_unique<trait_decl>()};
		
		ast->x = this->x;
		ast->y = this->y;

		this->next();

		if (this->peek(atom::SYMBOL))
		{
			ast->name = // move
			this->peek()->data;

			this->next();
		}
		else throw E(u8"expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '{'");

		// TODO

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '}'");

		return ast;
	}

	inline constexpr auto _stmt() -> std::optional<stmt>
	{
		try
		{
			if (auto tkn {this->peek()})
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
					case atom::L_PAREN:
					{
						return this->stmt_block();
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
						return this->stmt_iterate();
					}
				}
			}
			return std::nullopt;
		}
		catch (error<A, B>& out)
		{
			// recovery
			this->sync();
			// diagnostics
			this->exe.lint
			.emplace_back(out);
		}
		return this->_stmt();
	}

	inline constexpr auto stmt_if() -> stmt
	{
		auto ast {std::make_unique<if_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		_case_:
		// fresh construct
		if_stmt::flow data;

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects '('");

		if (auto out {this->_expr()})
		{
			data._if_ = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects ')'");

		_else_:
		if (auto out {this->_stmt()})
		{
			data.then = std::move(*out);
		}
		else throw E(u8"invalid stmt");

		//|--------------<insert>--------------|
		ast->body.emplace_back(std::move(data));
		//|------------------------------------|

		if (this->peek(atom::ELSE))
		{
			if (this->next(atom::IF))
			{
				this->next();
				goto _else_;
			}
			goto _case_;
		}
 
		return ast;
	}

	inline constexpr auto stmt_for() -> stmt
	{
		auto ast {std::make_unique<for_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects '('");

		if (auto out {this->_expr()})
		{
			ast->init = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"expects ';'");

		if (auto out {this->_expr()})
		{
			ast->_if_ = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"expects ';'");

		if (auto out {this->_expr()})
		{
			ast->task = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects ')'");

		if (auto out {this->_stmt()})
		{
			ast->body = std::move(*out);
		}
		else throw E(u8"invalid stmt");

		return ast;
	}

	inline constexpr auto stmt_match() -> stmt
	{
		auto ast {std::make_unique<match_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects '('");

		if (auto out {this->_expr()})
		{
			ast->data = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '{'");

		_case_:
		// fresh construct
		match_stmt::flow data;

		if (auto out {this->_expr()})
		{
			data._if_ = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"expects ':'");

		_else_:

		if (auto out {this->_stmt()})
		{
			data.then = std::move(*out);
		}
		else throw E(u8"invalid stmt");

		//|--------------<insert>--------------|
		ast->body.emplace_back(std::move(data));
		//|------------------------------------|

		if (this->peek(atom::CASE))
		{
			this->next();
			goto _case_;
		}
		if (this->peek(atom::ELSE))
		{
			this->next();
			goto _else_;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '}'");

		return ast;
	}

	inline constexpr auto stmt_while() -> stmt
	{
		auto ast {std::make_unique<while_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects '('");

		if (auto out {this->_expr()})
		{
			ast->_if_ = std::move(*out);
		}
		else throw E(u8"invalid expr");

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"expects ')'");

		if (auto out {this->_stmt()})
		{
			ast->body = std::move(*out);
		}
		else throw E(u8"invalid stmt");

		return ast;
	}

	inline constexpr auto stmt_block() -> stmt
	{
		auto ast {std::make_unique<block_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '{'");

		while (true)
		{
			if (auto out {this->_decl()})
			{
				std::visit([&](auto&& ptr)
				{
					ast->body.emplace_back(std::forward<decltype(ptr)>(ptr));
				},
				std::move(*out)); // unwrap
				continue;
			}
			if (auto out {this->_stmt()})
			{
				std::visit([&](auto&& ptr)
				{
					ast->body.emplace_back(std::forward<decltype(ptr)>(ptr));
				},
				std::move(*out)); // unwrap
				continue;
			}
			if (auto out {this->_expr()})
			{
				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"expects ';'");

				std::visit([&](auto&& ptr)
				{
					ast->body.emplace_back(std::forward<decltype(ptr)>(ptr));
				},
				std::move(*out)); // unwrap
				continue;
			}
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"expects '}'");

		return ast;
	}

	inline constexpr auto stmt_break() -> stmt
	{
		auto ast {std::make_unique<break_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		if (this->peek(atom::SYMBOL))
		{
			ast->label = // move
			this->peek()->data;

			this->next();
		}
		// else throw E(u8"expects 'ƒ'");

		return ast;
	}

	inline constexpr auto stmt_return() -> stmt
	{
		auto ast {std::make_unique<return_stmt>()};

		ast->x = this->x;
		ast->y = this->y;

		this->next();

		if (auto out {this->_expr()})
		{
			ast->value = std::move(*out);
		}
		// else throw E(u8"invalid expr");

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"expects ';'");

		return ast;
	}

	inline constexpr auto stmt_iterate() -> stmt
	{
		auto ast {std::make_unique<iterate_stmt>()};

		ast->x = this->x;
		ast->y = this->y;
		
		this->next();

		if (this->peek(atom::SYMBOL))
		{
			ast->label = // move
			this->peek()->data;

			this->next();
		}
		// else throw E(u8"expects 'ƒ'");

		return ast;
	}

	inline constexpr auto _expr(uint8_t mbp = 0) -> std::optional<expr>
	{
		try
		{
			expr lhs;

			if (auto tkn {this->peek()})
			{
				// null denoation
				if (auto power {parser::get_prefix_binding_power(tkn->type)})
				{
					auto [     rbp] {*power};

					auto ast {std::make_unique<prefix_expr>()};

					ast->x = this->x;
					ast->y = this->y;

					this->next();

					if (auto out {this->_expr()})
					{
						switch (tkn->type)
						{
							#define macro(K, V) \
							case atom::K:       \
							{                   \
							    ast->op = op::K;\
							    break;          \
							}                   \

							operators(macro)
							#undef macro

							default:
							{
								assert(!"<ERROR>");
								std::unreachable();
							}
						}
						ast->rhs = std::move(*out);
					}
					else throw E(u8"invalid expr");

					lhs = std::move(ast);
				}
				else
				{
					// handle primary
					if (auto out {this->expr_group()})
					{
						lhs = std::move(*out);
						goto LED;
					}
					if (auto out {this->expr_symbol()})
					{
						lhs = std::move(*out);
						goto LED;
					}
					if (auto out {this->expr_literal()})
					{
						lhs = std::move(*out);
						goto LED;
					}
					return std::nullopt;
				}

				LED:
				// left denotation
				while (auto tkn {this->peek()})
				{
					if (auto power {parser::get_infix_binding_power(tkn->type)})
					{
						auto [lbp, rbp] {*power};
						if (rbp < mbp) break;

						auto ast {std::make_unique<binary_expr>()};

						ast->x = this->x;
						ast->y = this->y;

						this->next();

						if (auto out {this->_expr()})
						{
							switch (tkn->type)
							{
								#define macro(K, V) \
								case atom::K:       \
								{                   \
									ast->op = op::K;\
									break;          \
								}                   \

								operators(macro)
								#undef macro

								default:
								{
									assert(!"<ERROR>");
									std::unreachable();
								}
							}
							ast->lhs = std::move(lhs);
							ast->rhs = std::move(*out);
						}
						else throw E(u8"invalid expr");

						lhs = std::move(ast);
						continue;
					}
					if (auto power {parser::get_suffix_binding_power(tkn->type)})
					{
						auto [     rbp] {*power};
						if (rbp < mbp) break;

						// handle suffix
						if (auto out {this->expr_access()})
						{
							lhs = std::move(*out);
							continue;
						}
						if (auto out {this->expr_invoke()})
						{
							lhs = std::move(*out);
							continue;
						}
						throw E(u8"parselet ???");
					}
					break;
				}
			}
			return lhs;
		}
		catch (error<A, B>& out)
		{
			// recovery
			this->sync();
			// diagnostics
			this->exe.lint
			.emplace_back(out);
		}
		return this->_expr();
	}

	//|-----------------------|
	//| primary expr parselet |
	//|-----------------------|

	inline constexpr auto expr_literal() -> std::optional<expr>
	{
		if (this->peek(atom::TRUE))
		{
			auto ast {std::make_unique<literal_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			ast->type = ty::BOOL;

			return ast;
		}
		if (this->peek(atom::FALSE))
		{
			auto ast {std::make_unique<literal_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			ast->type = ty::BOOL;

			return ast;
		}
		if (this->peek(atom::CODE))
		{
			auto ast {std::make_unique<literal_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			ast->type = ty::CODE;

			return ast;
		}
		if (this->peek(atom::TEXT))
		{
			auto ast {std::make_unique<literal_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			ast->type = ty::TEXT;

			return ast;
		}
		if (this->peek(atom::INT))
		{
			auto ast {std::make_unique<literal_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			ast->type = ty::I32;

			return ast;
		}
		if (this->peek(atom::DEC))
		{
			auto ast {std::make_unique<literal_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			ast->type = ty::F32;

			return ast;
		}
		return std::nullopt;
	}

	inline constexpr auto expr_symbol() -> std::optional<expr>
	{
		if (this->peek(atom::SYMBOL))
		{
			auto ast {std::make_unique<symbol_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			ast->self = // move
			this->peek()->data;

			this->next();

			return ast;
		}
		return std::nullopt;
	}

	inline constexpr auto expr_group() -> std::optional<expr>
	{
		if (this->peek(atom::L_PAREN))
		{
			auto ast {std::make_unique<group_expr>()};

			ast->x = this->x;
			ast->y = this->y;

			this->next();

			if (auto out {this->_expr()})
			{
				ast->self = std::move(*out);
			}
			else throw E(u8"invalid expr");

			if (this->peek(atom::R_PAREN))
			{
				this->next();
			}
			else throw E(u8"expects ')'");

			return ast;
		}
		return std::nullopt;
	}

	//|----------------------|
	//| suffix expr parselet |
	//|----------------------|

	inline constexpr auto expr_access() -> std::optional<expr>
	{
		return std::nullopt;
	}

	inline constexpr auto expr_invoke() -> std::optional<expr>
	{
		return std::nullopt;
	}

	//|---------------------|
	//| binding power table |
	//|---------------------|

	static constexpr auto get_prefix_binding_power(const atom type) -> std::optional<std::array<uint8_t, 1>>
	{
		switch (type)
		{
			case atom::POINT:
			case atom::DEREF:
			case atom::L_NOT:
			case atom::B_NOT:
			// prefix ver.
			case atom::ADD:
			case atom::SUB:
			{
				return {{69}};
			}
		}
		return std::nullopt;
	}

	static constexpr auto get_infix_binding_power(const atom type) -> std::optional<std::array<uint8_t, 2>>
	{
		switch (type)
		{
			case atom::ASSIGN:
			case atom::ADD_EQ:
			case atom::SUB_EQ:
			case atom::MUL_EQ:
			case atom::DIV_EQ:
			case atom::MOD_EQ:
			case atom::POW_EQ:
			{
				return {{1, 2}};
			}
			case atom::NIL:
			{
				return {{3, 4}};
			}
			case atom::B_OR:
			case atom::L_OR:
			{
				return {{5, 6}};
			}
			case atom::B_AND:
			case atom::L_AND:
			{
				return {{7, 8}};
			}
			case atom::EQ:
			case atom::NE:
			{
				return {{9, 10}};
			}
			case atom::LT:
			case atom::GT:
			case atom::LTE:
			case atom::GTE:
			{
				return {{11, 12}};
			}
			case atom::SHL:
			case atom::SHR:
			{
				return {{13, 14}};
			}
			case atom::ADD:
			case atom::SUB:
			{
				return {{15, 16}};
			}
			case atom::MUL:
			case atom::DIV:
			case atom::MOD:
			{
				return {{17, 18}};
			}
			case atom::POW:
			{
				return {{19, 18}};
			}
		}
		return std::nullopt;
	}

	static constexpr auto get_suffix_binding_power(const atom type) -> std::optional<std::array<uint8_t, 1>>
	{
		switch (type)
		{
			// expr_access
			case atom::ACCESS:
			// expr_invoke
			case atom::L_PAREN:
			{
				return {{69}};
			}
		}
		return std::nullopt;
	}
};
