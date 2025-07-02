#pragma once

#include <array>
#include <memory>
#include <cassert>
#include <cstdint>
#include <utility>
#include <variant>
#include <optional>
#include <iostream>
#include <functional>
#include <type_traits>

#include "./lexer.hpp"

#include "lang/common/ast.hpp"
#include "lang/common/eof.hpp"
#include "lang/common/token.hpp"
#include "lang/common/error.hpp"

template
<
	model::text A,
	model::text B
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

	program<A, B> exe;

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

	inline constexpr auto pull() -> program<A, B>
	{
		while (this->peek())
		{
			try
			{
				if (auto out {this->decl_t()})
				{
					//|-----------------<insert>-----------------|
					std::visit([&](auto&& _)
					{
						this->exe.entry.emplace_back(std::move(_));
					},
					std::move(out.value()));
					//|------------------------------------------|
					continue;
				}
				if (auto out {this->stmt_t()})
				{
					//|-----------------<insert>-----------------|
					std::visit([&](auto&& _)
					{
						this->exe.entry.emplace_back(std::move(_));
					},
					std::move(out.value()));
					//|------------------------------------------|
					continue;
				}
				throw E(u8"[parser] unknown decl/stmt");
			}
			catch (error<A, B>& out)
			{
				//|----------------<insert>----------------|
				this->exe.issue.emplace_back(std::move(out));
				//|----------------------------------------|
				this->sync(); // ...nothing to do here...
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

	//|--------------|
	//| declarations |
	//|--------------|

	inline constexpr auto decl_t() -> std::optional<decl>
	{
		try
		{
			if (auto tkn {this->peek()})
			{
				switch (tkn->type)
				{
					case atom::FUN:
					{
						return this->decl_fun();
					}
					case atom::FUN$:
					{
						return this->decl_fun$();
					}
					case atom::LET:
					{
						return this->decl_var();
					}
					case atom::LET$:
					{
						return this->decl_var$();
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
				return std::nullopt;
			}
		}
		catch (error<A, B>& out)
		{
			//|----------------<insert>----------------|
			this->exe.issue.emplace_back(std::move(out));
			//|----------------------------------------|
			this->sync(); return this->decl_t();
		}
		assert(false && "-Wreturn-type");
	}

	inline constexpr auto decl_fun() -> decltype(this->decl_t())
	{
		$fun ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		//|----<update>----|
		ast.is_pure = false;
		//|----------------|

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.name = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");
	
		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		start:
		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();
			
			$var arg;

			//|---------<update>---------|
			arg.name = std::move(tkn.data);
			//|--------------------------|

			if (this->peek(atom::COLON))
			{
				this->next();
			}
			else throw E(u8"[parser] expects ':'");

			if (this->peek(atom::SYMBOL))
			{
				//|----------<copy>----------|
				const auto tkn {*this->peek()};
				//|--------------------------|

				this->next();

				//|---------<update>---------|
				arg.type = std::move(tkn.data);
				//|--------------------------|
			}
			else throw E(u8"[parser] expects 'ƒ'");

			if (this->peek(atom::ASSIGN))
			{
				this->next();

				//|--------------<catch>--------------|
				arg.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{throw E(u8"[parser] invalid expr");});
				//|-----------------------------------|
			}
			// else throw E(u8"[parser] expects '='");

			//|-------------<insert>-------------|
			ast.args.emplace_back(std::move(arg));
			//|----------------------------------|
			
			if (this->peek(atom::COMMA))
			{
				this->next();
				goto start;
			}
			// else throw E(u8"[parser] expects ','";)
		}

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.type = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|-------------<insert>-------------|
				std::visit([&](auto&& _)
				{
					ast.body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|----------------------------------|
				continue;
			}
			if (auto out {this->decl_t()})
			{
				//|-------------<insert>-------------|
				std::visit([&](auto&& _)
				{
					ast.body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|----------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|-------------<insert>-------------|
				std::visit([&](auto&& _)
				{
					ast.body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|----------------------------------|
	
				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] expects ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto decl_fun$() -> decltype(this->decl_t())
	{
		$fun ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		//|---<update>---|
		ast.is_pure = true;
		//|---------------|

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.name = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");
	
		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		start:
		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();
			
			$var arg;

			//|---------<update>---------|
			arg.name = std::move(tkn.data);
			//|--------------------------|

			if (this->peek(atom::COLON))
			{
				this->next();
			}
			else throw E(u8"[parser] expects ':'");

			if (this->peek(atom::SYMBOL))
			{
				//|----------<copy>----------|
				const auto tkn {*this->peek()};
				//|--------------------------|

				this->next();

				//|---------<update>---------|
				arg.type = std::move(tkn.data);
				//|--------------------------|
			}
			else throw E(u8"[parser] expects 'ƒ'");

			if (this->peek(atom::ASSIGN))
			{
				this->next();

				//|--------------<catch>--------------|
				arg.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{throw E(u8"[parser] invalid expr");});
				//|-----------------------------------|
			}
			// else throw E(u8"[parser] expects '='");

			//|-------------<insert>-------------|
			ast.args.emplace_back(std::move(arg));
			//|----------------------------------|
			
			if (this->peek(atom::COMMA))
			{
				this->next();
				goto start;
			}
			// else throw E(u8"[parser] expects ','";)
		}

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.type = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|-------------<insert>-------------|
				std::visit([&](auto&& _)
				{
					ast.body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|----------------------------------|
				continue;
			}
			if (auto out {this->decl_t()})
			{
				//|-------------<insert>-------------|
				std::visit([&](auto&& _)
				{
					ast.body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|----------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|-------------<insert>-------------|
				std::visit([&](auto&& _)
				{
					ast.body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|----------------------------------|
	
				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] expects ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto decl_var() -> decltype(this->decl_t())
	{
		$var ast;
		
		ast.x = this->x;
		ast.y = this->y;

		this->next();

		//|----<update>----|
		ast.is_const = false;
		//|----------------|

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.name = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.type = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::ASSIGN))
		{
			this->next();

			//|--------------<catch>--------------|
			ast.init = *this->expr_t().or_else([&]
				-> decltype(this->expr_t())
			{throw E(u8"[parser] invalid expr");});
			//|-----------------------------------|
		}
		// else throw u8"[parser] must init";

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ';'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto decl_var$() -> decltype(this->decl_t())
	{
		$var ast;
		
		ast.x = this->x;
		ast.y = this->y;

		this->next();

		//|----<update>----|
		ast.is_const = true;
		//|----------------|

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.name = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.type = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::ASSIGN))
		{
			this->next();

			//|--------------<catch>--------------|
			ast.init = *this->expr_t().or_else([&]
				-> decltype(this->expr_t())
			{throw E(u8"[parser] invalid expr");});
			//|-----------------------------------|
		}
		else throw E(u8"[parser] must init");

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ';'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto decl_model() -> decltype(this->decl_t())
	{
		$model ast;
		
		ast.x = this->x;
		ast.y = this->y;

		this->next();

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.name = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		// <body>
		while (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();
			
			$var arg;

			//|---------<update>---------|
			arg.name = std::move(tkn.data);
			//|--------------------------|

			if (this->peek(atom::COLON))
			{
				this->next();
			}
			else throw E(u8"[parser] expects ':'");

			if (this->peek(atom::SYMBOL))
			{
				//|----------<copy>----------|
				const auto tkn {*this->peek()};
				//|--------------------------|

				this->next();

				//|---------<update>---------|
				arg.type = std::move(tkn.data);
				//|--------------------------|
			}
			else throw E(u8"[parser] expects 'ƒ'");

			if (this->peek(atom::ASSIGN))
			{
				this->next();

				//|--------------<catch>--------------|
				arg.init = *this->expr_t().or_else([&]
					-> decltype(this->expr_t())
				{throw E(u8"[parser] invalid expr");});
				//|-----------------------------------|
			}
			// else throw E(u8"[parser] expects '='");

			if (this->peek(atom::S_COLON))
			{
				this->next();
			}
			else throw E(u8"[parser] expects ';'");

			//|-------------<insert>-------------|
			ast.body.emplace_back(std::move(arg));
			//|----------------------------------|
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto decl_trait() -> decltype(this->decl_t())
	{
		$trait ast;
		
		ast.x = this->x;
		ast.y = this->y;

		this->next();

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|---------<update>---------|
			ast.name = std::move(tkn.data);
			//|--------------------------|
		}
		else throw E(u8"[parser] expects 'ƒ'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		while (true)
		{
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	//|------------|
	//| statements |
	//|------------|

	inline constexpr auto stmt_t() -> std::optional<stmt>
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
				return std::nullopt;
			}
		}
		catch (error<A, B>& out)
		{
			//|----------------<insert>----------------|
			this->exe.issue.emplace_back(std::move(out));
			//|----------------------------------------|
			this->sync(); return this->stmt_t();
		}
		assert(false && "-Wreturn-type");
	}

	inline constexpr auto stmt_if() -> decltype(this->stmt_t())
	{
		$if ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		expr expr;
		body body;
		
		else_if:
		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		//|--------------<catch>--------------|
			expr = {*this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");})};
		//|-----------------------------------|

		//|--------------<insert>--------------|
		ast.cases.emplace_back(std::move(expr));
		//|------------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if_block:
		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|-----------<insert>-----------|
				std::visit([&](auto&& _)
				{
					body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|-----------<insert>-----------|
				std::visit([&](auto&& _)
				{
					body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] expects ';'");
				// again..!
				continue;
			}
			break;
		}

		//|--------------<insert>--------------|
		ast.block.emplace_back(std::move(body));
		//|------------------------------------|

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		if (this->peek(atom::ELSE))
		{
			if (this->next(atom::IF))
			{
				this->next();
				goto else_if;
			}
			goto if_block;
		}

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto stmt_for() -> decltype(this->stmt_t())
	{
		$for ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		//|--------------<catch>--------------|
		ast.setup = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");});
		//|-----------------------------------|

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ';'");

		//|--------------<catch>--------------|
		ast.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");});
		//|-----------------------------------|

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ';'");

		//|--------------<catch>--------------|
		ast.after = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");});
		//|-----------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|--------------<insert>--------------|
				std::visit([&](auto&& _)
				{
					ast.block.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|--------------<insert>--------------|
				std::visit([&](auto&& _)
				{
					ast.block.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] expects ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto stmt_match() -> decltype(this->stmt_t())
	{
		$match ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		expr expr;
		body body;

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		//|--------------<catch>--------------|
		ast.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");});
		//|-----------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		if (this->peek(atom::CASE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects 'case'");

		case_block:
		//|--------------<catch>--------------|
			expr = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");});
		//|-----------------------------------|

		//|--------------<insert>--------------|
		ast.cases.emplace_back(std::move(expr));
		//|------------------------------------|

		else_block:
		if (this->peek(atom::COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ':'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|-----------<insert>-----------|
				std::visit([&](auto&& _)
				{
					body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|-----------<insert>-----------|
				std::visit([&](auto&& _)
				{
					body.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] expects ';'");
				// again..!
				continue;
			}
			break;
		}

		//|--------------<insert>--------------|
		ast.block.emplace_back(std::move(body));
		//|------------------------------------|

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");

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
		else throw E(u8"[parser] expects '}'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto stmt_while() -> decltype(this->stmt_t())
	{
		$while ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		if (this->peek(atom::L_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '('");

		//|--------------<catch>--------------|
		ast.input = *this->expr_t().or_else([&]
			-> decltype(this->expr_t())
		{throw E(u8"[parser] invalid expr");});
		//|-----------------------------------|

		if (this->peek(atom::R_PAREN))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ')'");

		if (this->peek(atom::L_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '{'");

		// <block>
		while (true)
		{
			if (auto out {this->stmt_t()})
			{
				//|--------------<insert>--------------|
				std::visit([&](auto&& _)
				{
					ast.block.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------------|
				continue;
			}
			if (auto out {this->expr_t()})
			{
				//|--------------<insert>--------------|
				std::visit([&](auto&& _)
				{
					ast.block.emplace_back(std::move(_));
				},
				std::move(out.value()));
				//|------------------------------------|

				if (this->peek(atom::S_COLON))
				{
					this->next();
				}
				else throw E(u8"[parser] expects ';'");
				// again..!
				continue;
			}
			break;
		}

		if (this->peek(atom::R_BRACE))
		{
			this->next();
		}
		else throw E(u8"[parser] expects '}'");
	
		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto stmt_break() -> decltype(this->stmt_t())
	{
		$break ast;

		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			ast.label = std::move(tkn.data);
			//|----------------------------|
		}

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto stmt_return() -> decltype(this->stmt_t())
	{
		$return ast;

		ast.x = this->x;
		ast.y = this->y;

		this->next();

		if (auto out {this->expr_t()})
		{
			//|-------<update>--------|
			ast.value = std::move(*out);
			//|-----------------------|
		}

		if (this->peek(atom::S_COLON))
		{
			this->next();
		}
		else throw E(u8"[parser] expects ';'");

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	inline constexpr auto stmt_continue() -> decltype(this->stmt_t())
	{
		$continue ast;
		
		ast.x = this->x;
		ast.y = this->y;
		
		this->next();

		if (this->peek(atom::SYMBOL))
		{
			//|----------<copy>----------|
			const auto tkn {*this->peek()};
			//|--------------------------|

			this->next();

			//|----------<update>----------|
			ast.label = std::move(tkn.data);
			//|----------------------------|
		}

		return std::make_unique
		<decltype(ast)>(std::move(ast));
	}

	//|-------------|
	//| expressions |
	//|-------------|

	inline constexpr auto expr_t() -> std::optional<expr> // pratt parser
	{
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
							if (is_l(*tkn))
							{
								$unary ast;
								
								ast.x = this->x;
								ast.y = this->y;

								this->next();

								//|--------------<catch>--------------|
								std::optional rhs {impl(69).or_else([&]
									-> decltype(this->expr_t())
								{throw E(u8"[parser] invalid expr");})};
								//|-----------------------------------|

								ast.opr = to_l(*tkn);
								ast.rhs = std::move(*rhs);

								return std::make_unique
								<decltype(ast)>(std::move(ast));
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
						if (is_i(*tkn))
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
											assert(!"[ERROR]");
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

							$binary ast;

							ast.x = this->x;
							ast.y = this->y;
							
							this->next();

							//|--------------<catch>--------------|
							std::optional rhs {impl(rbp).or_else([&]
								-> decltype(this->expr_t())
							{throw E(u8"[parser] invalid expr");})};
							//|-----------------------------------|

							ast.opr = to_i(*tkn);
							ast.lhs = std::move(*lhs);
							ast.rhs = std::move(*rhs);

							lhs = std::make_unique
							<decltype(ast)>(std::move(ast));
							continue;
						}

						// handle postfix
						if (is_r(*tkn))
						{
							$access ast;

							ast.x = this->x;
							ast.y = this->y;
							
							this->next();

							if (this->peek(atom::SYMBOL))
							{
								//|----------<copy>----------|
								const auto tkn {*this->peek()};
								//|--------------------------|

								this->next();

								//|----------<update>----------|
								ast.name = std::move(tkn.data);
								//|----------------------------|
							}
							else throw E(u8"[parser] expects 'ƒ'");

							ast.type = to_r(*tkn);
							ast.expr = std::move(*lhs);
							
							lhs = std::make_unique
							<decltype(ast)>(std::move(ast));
							continue;
						}

						// handle function
						if (tkn == atom::L_PAREN)
						{
							$call ast;

							ast.x = this->x;
							ast.y = this->y;

							this->next();

							//|--------<update>--------|
							ast.self = std::move(*lhs);
							//|------------------------|

							start:
							if (auto out {this->expr_t()})
							{
								//|--------------<insert>--------------|
								ast.args.emplace_back(std::move(*out));
								//|------------------------------------|

								if (this->peek(atom::COMMA))
								{
									this->next();
									goto start;
								}
								// else throw u8"[parser] expects ','";
							}

							if (this->peek(atom::R_PAREN))
							{
								this->next();
							}
							else throw E(u8"[parser] expects ')'");

							lhs = std::make_unique
							<decltype(ast)>(std::move(ast));
							continue;
						}
						break;
					}
				}
				return lhs;
			}
		};
		try
		{
			return impl(0);
		}
		catch (error<A, B>& out)
		{
			//|----------------<insert>----------------|
			this->exe.issue.emplace_back(std::move(out));
			//|----------------------------------------|
			this->sync(); return this->expr_t();
		}
		assert(false && "-Wreturn-type");
	}

	inline constexpr auto expr_group() -> decltype(this->expr_t())
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::L_PAREN:
				{
					$group ast;

					ast.x = this->x;
					ast.y = this->y;
					
					this->next();

					//|--------------<catch>--------------|
					ast.expr = *this->expr_t().or_else([&]
						-> decltype(this->expr_t())
					{throw E(u8"[parser] invalid expr");});
					//|-----------------------------------|

					if (this->peek(atom::R_PAREN))
					{
						this->next();
					}
					else throw E(u8"[parser] expects ')'");

					return std::make_unique
					<decltype(ast)>(std::move(ast));
				}
			}
		}
		return std::nullopt;
	}

	inline constexpr auto expr_symbol() -> decltype(this->expr_t())
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

					return std::make_unique
					<decltype(node)>(std::move(node));
				}
			}
		}
		return std::nullopt;
	}

	inline constexpr auto expr_literal() -> decltype(this->expr_t())
	{
		if (auto tkn {this->peek()})
		{
			switch (tkn->type)
			{
				case atom::TRUE:
				case atom::FALSE:
				{
					$literal ast;

					ast.x = this->x;
					ast.y = this->y;
					
					this->next();

					//|----------<update>----------|
					ast.type = type::BOOL;
					ast.data = std::move(tkn->data);
					//|----------------------------|

					return std::make_unique
					<decltype(ast)>(std::move(ast));
				}
				case atom::CODE:
				{
					$literal ast;

					ast.x = this->x;
					ast.y = this->y;
					
					this->next();

					//|----------<update>----------|
					ast.type = type::CODE;
					ast.data = std::move(tkn->data);
					//|----------------------------|

					return std::make_unique
					<decltype(ast)>(std::move(ast));
				}
				case atom::TEXT:
				{
					$literal ast;

					ast.x = this->x;
					ast.y = this->y;
					
					this->next();

					//|----------<update>----------|
					ast.type = type::TEXT;
					ast.data = std::move(tkn->data);
					//|----------------------------|

					return std::make_unique
					<decltype(ast)>(std::move(ast));
				}
				case atom::DEC:
				{
					$literal ast;

					ast.x = this->x;
					ast.y = this->y;
					
					this->next();

					//|----------<update>----------|
					ast.type = type::F32;
					ast.data = std::move(tkn->data);
					//|----------------------------|

					return std::make_unique
					<decltype(ast)>(std::move(ast));
				}
				case atom::INT:
				case atom::BIN:
				case atom::OCT:
				case atom::HEX:
				{
					$literal ast;

					ast.x = this->x;
					ast.y = this->y;
					
					this->next();

					//|----------<update>----------|
					ast.type = type::I32;
					ast.data = std::move(tkn->data);
					//|----------------------------|

					return std::make_unique
					<decltype(ast)>(std::move(ast));
				}
			}
		}
		return std::nullopt;
	}
};
