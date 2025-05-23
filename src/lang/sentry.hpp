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
		std::vector<lang::$var*> $var;
		std::vector<lang::$fun*> $fun;
		std::vector<lang::$trait*> $trait;
		std::vector<lang::$class*> $class;
	};

	struct checker : lang::visitor
	{
		std::vector<context> ctx;

	public:

		checker(decltype(ctx) ctx) : ctx {ctx} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		void visit(const only<decl>& data) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			data);
		}

		void visit(const many<decl>& data) override
		{
			for (auto&& node : data)
			{
				this->visit(node);
			}
		}

		void visit(const only<stmt>& data) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			data);
		}

		void visit(const many<stmt>& data) override
		{
			for (auto&& node : data)
			{
				this->visit(node);
			}
		}

		void visit(const only<expr>& data) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			data);
		}

		void visit(const many<expr>& data) override
		{
			for (auto&& node : data)
			{
				this->visit(node);
			}
		}

		void visit(const only<node>& data) override
		{
			std::visit([&](auto&& arg)
			{
				this->visit(arg);
			},
			data);
		}

		void visit(const many<node>& data) override
		{
			for (auto&& node : data)
			{
				this->visit(node);
			}
		}

		void visit(const many<body>& data) override
		{
			for (auto&& node : data)
			{
				this->visit(node);
			}
		}

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit(lang::$var& data) override
		{

		}

		void visit(lang::$fun& data) override
		{

		}

		void visit(lang::$trait& data) override
		{

		}

		void visit(lang::$class& data) override
		{

		}

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit(lang::$if& data) override
		{

		}

		void visit(lang::$match& data) override
		{

		}

		void visit(lang::$for& data) override
		{

		}

		void visit(lang::$while& data) override
		{

		}

		void visit(lang::$break& data) override
		{

		}

		void visit(lang::$return& data) override
		{

		}

		void visit(lang::$continue& data) override
		{

		}

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit(lang::$unary& data) override
		{

		}

		void visit(lang::$binary& data) override
		{
			
		}

		void visit(lang::$literal& data) override
		{
			
		}

		void visit(lang::$symbol& data) override
		{

		}

		void visit(lang::$access& data) override
		{
			
		}

		void visit(lang::$group& data) override
		{
			
		}

		void visit(lang::$call& data) override
		{
			
		}
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
				std::visit([&](auto&& arg)
				{
					impl.visit(arg);
				},
				node);
			}
		}
		return this->buffer;
	}
};
