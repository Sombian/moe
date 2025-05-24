#pragma once

#include <map>
#include <vector>
#include <optional>

#include "lang/parser.hpp"

#include "lang/common/ast.hpp"

template
<
	type::string A,
	type::string B
>
class sentry
{
	parser<A, B>* parser;

	//|----------<buffer>----------|
	decltype(parser->pull()) buffer;
	//|----------------------------|

	struct context
	{
		bool isolate;

		std::map<utf8, lang::$var*> $var;
		std::map<utf8, lang::$fun*> $fun;
		
		std::map<utf8, lang::$trait*> $trait;
		std::map<utf8, lang::$class*> $class;
	};

	struct checker : lang::visitor
	{
		std::vector<context> ctx;

	public:

		checker(decltype(ctx) ctx) : ctx {ctx} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		void visit(const only<decl>& ast) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many<decl>& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only<stmt>& ast) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many<stmt>& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only<expr>& ast) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many<expr>& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only<node>& ast) override
		{
			std::visit([&](auto&& arg)
			{
				this->visit(arg);
			},
			ast);
		}

		void visit(const many<node>& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const many<body>& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		#define VISIT($T)           \
		                            \
		void visit(many<$T>& list)  \
		{                           \
			for (auto&& ast : list) \
			{                       \
				this->visit(ast);   \
			}                       \
		}                           \

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit(lang::$var& ast) override
		{
			// try to insert
			auto [pair, insert] {this->ctx.back().$var.try_emplace(ast.name, &ast)};

			// visit children
			this->visit(ast.init);

			if (!insert)
			{
				std::cout << "[sentry] redefinition of var '" << ast.name << "'" << std::endl;
			}
		}
		VISIT(lang::$var)

		void visit(lang::$fun& ast) override
		{
			// try to insert
			auto [pair, insert] {this->ctx.back().$fun.try_emplace(ast.name, &ast)};

			// visit children
			this->visit(ast.args);
			this->visit(ast.body);

			if (!insert)
			{
				std::cout << "[sentry] redefinition of fun '" << ast.name << "'" << std::endl;
			}
		}
		VISIT(lang::$fun)

		void visit(lang::$trait& ast) override
		{
			// try to insert
			auto [pair, insert] {this->ctx.back().$trait.try_emplace(ast.name, &ast)};

			// visit children
			this->visit(ast.body);

			if (!insert)
			{
				std::cout << "[sentry] redefinition of trait '" << ast.name << "'" << std::endl;
			}
		}
		VISIT(lang::$trait)

		void visit(lang::$class& ast) override
		{
			// try to insert
			auto [pair, insert] {this->ctx.back().$class.try_emplace(ast.name, &ast)};

			// visit children
			this->visit(ast.body);

			if (!insert)
			{
				std::cout << "[sentry] redefinition of class '" << ast.name << "'" << std::endl;
			}
		}
		VISIT(lang::$class)

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit(lang::$if& ast) override
		{
			this->visit(ast.cases);
			this->visit(ast.block);
		}
		VISIT(lang::$if)

		void visit(lang::$match& ast) override
		{
			this->visit(ast.input);
			this->visit(ast.cases);
			this->visit(ast.block);
		}
		VISIT(lang::$match)

		void visit(lang::$for& ast) override
		{
			this->visit(ast.setup);
			this->visit(ast.input);
			this->visit(ast.after);
			this->visit(ast.block);
		}
		VISIT(lang::$for)

		void visit(lang::$while& ast) override
		{
			this->visit(ast.input);
			this->visit(ast.block);
		}
		VISIT(lang::$while)

		void visit(lang::$break& ast) override
		{
			// this->visit(ast.label);
		}
		VISIT(lang::$break)

		void visit(lang::$return& ast) override
		{
			this->visit(ast.value);
		}
		VISIT(lang::$return)

		void visit(lang::$continue& ast) override
		{
			// this->visit(ast.label);
		}
		VISIT(lang::$continue)

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit(lang::$unary& ast) override
		{
			this->visit(ast.rhs);
		}
		VISIT(lang::$unary)

		void visit(lang::$binary& ast) override
		{
			this->visit(ast.lhs);
			this->visit(ast.rhs);
		}
		VISIT(lang::$binary)

		void visit(lang::$literal& ast) override
		{
			
		}
		VISIT(lang::$literal)

		void visit(lang::$symbol& ast) override
		{
			
		}
		VISIT(lang::$symbol)

		void visit(lang::$access& ast) override
		{
			
		}
		VISIT(lang::$access)

		void visit(lang::$group& ast) override
		{
			
		}
		VISIT(lang::$group)

		void visit(lang::$call& ast) override
		{
			
		}
		VISIT(lang::$call)
	};

public:

	sentry
	(
		decltype(parser) parser
	)
	: parser {parser}, buffer {parser->pull()} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	operator fs::file<A, B>*()
	{
		return static_cast
		<fs::file<A, B>*>
		(*this->parser);
	}

	auto pull() -> std::optional<program>&
	{
		checker impl {{{}}};

		if (auto&& exe {this->buffer})
		{
			for (auto&& node : exe->ast)
			{
				try
				{
					std::visit([&](auto&& arg)
					{
						impl.visit(arg);
					},
					node);
				}
				catch (error<A, B>& error)
				{
					#ifndef NDEBUG //-------------|
					std::cout << error << std::endl;
					#endif //---------------------|
				}
			}
		}
		return this->buffer;
	}
};
