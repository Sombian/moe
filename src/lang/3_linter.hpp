#pragma once

#include <deque>
#include <vector>
#include <memory>
#include <variant>
#include <utility>
#include <optional>
#include <functional>
#include <type_traits>

#include "models/str.hpp"

#include "lang/2_parser.hpp"

#include "./common/ast.hpp"

template
<
	type::string A,
	type::string B
>
class linter
{
	friend class impl;

	parser<A, B>* parser;

	#define only(T) T /* sugar */
	#define many(T) std::deque<T>

	struct scope
	{
		only(scope*) upper {nullptr};
		many(scope*) lower {/*^-^*/};

		std::vector<std::variant
		<
			std::pair<utf8, const $var*>,
			std::pair<utf8, const $fun*>,
			std::pair<utf8, const $trait*>,
			std::pair<utf8, const $class*>
		>>
		define; // scalable..?

		~scope()
		{
			for (auto&& node : this->lower)
			{
				delete node; // free memory
			}
		}
	};

	scope* ctx {new scope {nullptr}};
	//|----------<buffer>----------|
	decltype(parser->pull()) buffer;
	//|----------------------------|

	template
	<
		typename T
	>
	static constexpr
	auto lookup(scope* ctx, const utf8& name) -> T
	{
		typedef std::pair<utf8, T> P;

		for (auto& node : ctx->define)
		{
			if (std::holds_alternative<P>(node))
			{
				auto [key, value] {std::get<P>(node)};

				if (key == name)
				{
					return value;
				}
			}
		}
		return nullptr;
	}

	template
	<
		typename T
	>
	static constexpr
	auto inject(scope* ctx, const T& ast) -> many(utf8)
	{
		many(utf8) report;

		for (auto&& plugin : RULES)
		{
			std::visit([&](auto&& fun)
			{
				typedef std::decay_t<decltype(fun)> F;

				if constexpr (std::is_invocable_v<F, scope*, T&>)
				{
					if (auto error {fun(ctx, ast)})
					{
						report.emplace_back(*error);
					}
				}
			},
			plugin);
		}
		return report;
	}

public:

	linter
	(
		decltype(parser) parser
	)
	: parser {parser}, buffer {parser->pull()}
	{
		class impl : public lang::reflect
		{
			#define START                            \
			{                                        \
				scope* node {new scope {this->ptr}}; \
				this->ptr->lower.emplace_back(node); \
				this->ptr = this->ptr->lower.back(); \
			}                                        \
			
			#define CLOSE                     \
			{                                 \
				this->ptr = this->ptr->upper; \
			}                                 \
			
			//|--<safe ptr--|
			linter<A, B>* src;
			//|-------------|
			mutable scope* ptr;
			
		public:

			using reflect::visit; // super

			impl // visitor pattern impl
			(
				decltype(src) src
			)
			: src {src}, ptr {src->ctx} {}
			
			//|---------------|
			//| variant::decl |
			//|---------------|

			void visit(const $var& ast) const override
			{
				// insert to the ctx
				{
					this->ptr->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.init);
			}

			void visit(const $fun& ast) const override
			{
				START;

				// insert to the ctx
				{
					this->ptr->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.args);
				this->visit(ast.body);

				CLOSE;
			}

			void visit(const $trait& ast) const override
			{
				// insert to the ctx
				{
					this->ptr->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.body);
			}

			void visit(const $class& ast) const override
			{
				START;

				// insert to the ctx
				{
					this->ptr->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.body);

				CLOSE;
			}

			#undef START
			#undef CLOSE
		};

		const impl core {this};
		this->buffer.run(core);
	}

	~linter()
	{
		delete this->ctx;
	}

	//|-----------------|
	//| member function |
	//|-----------------|

	operator fs::file<A, B>*()
	{
		return *this->parser;
	}

	auto pull() -> program<A, B>
	{
		class impl : public lang::reflect
		{	
			#define CHECK                                \
			{                                            \
				for (auto& msg : inject(this->ptr, ast)) \
				{                                        \
					this->src->buffer.fault.emplace_back \
					(                                    \
						ast.x, ast.y, *this->src, msg    \
					);                                   \
				}                                        \
			}                                            \

			#define START                        \
			{                                    \
				this->ptr = this->ptr->lower[0]; \
			}                                    \
			
			#define CLOSE                     \
			{                                 \
				this->ptr = this->ptr->upper; \
				this->ptr->lower.pop_front(); \
			}                                 \
			
			//|--<safe ptr--|
			linter<A, B>* src;
			//|-------------|
			mutable scope* ptr;

		public:

			using reflect::visit; // super

			impl // visitor pattern impl
			(
				decltype(src) src
			)
			: src {src}, ptr {src->ctx} {}

			//|---------------|
			//| variant::decl |
			//|---------------|

			void visit(const $var& ast) const override
			{
				CHECK;
				this->visit(ast.init);
			}

			void visit(const $fun& ast) const override
			{
				START;

				CHECK;
				this->visit(ast.args);
				this->visit(ast.body);
				
				CLOSE;
			}

			void visit(const $trait& ast) const override
			{
				CHECK;
				this->visit(ast.body);
			}

			void visit(const $class& ast) const override
			{
				START;

				CHECK;
				this->visit(ast.body);

				CLOSE;
			}

			#undef START
			#undef CLOSE

			//|---------------|
			//| variant::stmt |
			//|---------------|

			void visit(const $if& ast) const override
			{
				CHECK;
				this->visit(ast.cases);
				this->visit(ast.block);
			}

			void visit(const $for& ast) const override
			{ 
				CHECK;
				this->visit(ast.setup);
				this->visit(ast.input);
				this->visit(ast.after);
				this->visit(ast.block);
			}

			void visit(const $match& ast) const override
			{
				CHECK;
				this->visit(ast.input);
				this->visit(ast.cases);
				this->visit(ast.block);
			}

			void visit(const $while& ast) const override
			{
				CHECK;
				this->visit(ast.input);
				this->visit(ast.block);
			}

			void visit(const $break& ast) const override
			{
				CHECK;
			}

			void visit(const $return& ast) const override
			{
				CHECK;
				this->visit(ast.value);
			}

			void visit(const $continue& ast) const override
			{
				CHECK;
			}

			//|---------------|
			//| variant::expr |
			//|---------------|

			void visit(const $unary& ast) const override
			{
				CHECK;
				this->visit(ast.rhs);
			}

			void visit(const $binary& ast) const override 
			{
				CHECK;
				this->visit(ast.lhs);
				this->visit(ast.rhs);
			}

			void visit(const $literal& ast) const override
			{
				CHECK;
			}

			void visit(const $symbol& ast) const override
			{
				CHECK;
			}

			void visit(const $access& ast) const override
			{
				CHECK;
			}

			void visit(const $group& ast) const override
			{
				CHECK;
				this->visit(ast.expr);
			}

			void visit(const $call& ast) const override
			{
				CHECK;
				this->visit(ast.args);
				this->visit(ast.call);
			}

			#undef CHECK
		};

		const impl core {this};
		this->buffer.run(core);

		return std::move(this->buffer);
	}

	#undef only
	#undef many

private:

	static inline
	// define plugins
	const std::variant
	<
		//|---------------|
		//| variant::decl |
		//|---------------|
		std::optional<utf8>(*)(scope*, const $var&),
		std::optional<utf8>(*)(scope*, const $fun&),
		std::optional<utf8>(*)(scope*, const $trait&),
		std::optional<utf8>(*)(scope*, const $class&),
		//|---------------|
		//| variant::stmt |
		//|---------------|
		std::optional<utf8>(*)(scope*, const $if&),
		std::optional<utf8>(*)(scope*, const $for&),
		std::optional<utf8>(*)(scope*, const $match&),
		std::optional<utf8>(*)(scope*, const $while&),
		std::optional<utf8>(*)(scope*, const $break&),
		std::optional<utf8>(*)(scope*, const $return&),
		std::optional<utf8>(*)(scope*, const $continue&),
		//|---------------|
		//| variant::expr |
		//|---------------|
		std::optional<utf8>(*)(scope*, const $unary&),
		std::optional<utf8>(*)(scope*, const $binary&),
		std::optional<utf8>(*)(scope*, const $literal&),
		std::optional<utf8>(*)(scope*, const $symbol&),
		std::optional<utf8>(*)(scope*, const $access&),
		std::optional<utf8>(*)(scope*, const $group&),
		std::optional<utf8>(*)(scope*, const $call&)
	>
	RULES[]
	{
		[](scope* ctx, const $symbol& ast) -> std::optional<utf8>
		{
			for (auto _ctx {ctx}; _ctx; _ctx = _ctx->upper)
			{
				if (auto ptr {lookup<const $var*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
				if (auto ptr {lookup<const $fun*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
				if (auto ptr {lookup<const $trait*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
				if (auto ptr {lookup<const $class*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
			}
			return u8"cannot find definition for symbol '%s'" | ast.name;
		},
		[](scope* ctx, const $var& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<const $var*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"variable definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, const $fun& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<const $fun*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"function definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, const $trait& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<const $trait*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"trait definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, const $class& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<const $class*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"class definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, const $binary& ast) -> std::optional<utf8>
		{
			switch (ast.op)
			{
				case op_i::ASSIGN:
				case op_i::ADD_EQ:
				case op_i::SUB_EQ:
				case op_i::MUL_EQ: 
				case op_i::DIV_EQ:
				case op_i::MOD_EQ:
				case op_i::POW_EQ:
				{
					break;
				}
			}
			return std::nullopt;
		},
		[](scope* ctx, const $binary& ast) -> std::optional<utf8>
		{
			switch (ast.op)
			{
				case op_i::ASSIGN:
				case op_i::ADD_EQ:
				case op_i::SUB_EQ:
				case op_i::MUL_EQ:
				case op_i::DIV_EQ:
				case op_i::MOD_EQ:
				case op_i::POW_EQ:
				{
					break;
				}
			}
			return std::nullopt;
		},
		[](scope* ctx, const $binary& ast) -> std::optional<utf8>
		{
			switch (ast.op)
			{
				case op_i::ASSIGN:
				case op_i::ADD_EQ:
				case op_i::SUB_EQ:
				case op_i::MUL_EQ:
				case op_i::DIV_EQ:
				case op_i::MOD_EQ:
				case op_i::POW_EQ:
				{
					if (auto ptr {std::get_if<symbol_t>(&ast.lhs)})
					{
						for (auto _ctx {ctx}; _ctx; _ctx = _ctx->upper)
						{
							if (auto var {lookup<const $var*>(_ctx, (*ptr)->name)})
							{
								if (*var < ast)
								{
									if (!var->is_const)
									{
										return std::nullopt;
									}
									return u8"cannot assign to constant variable '%s'" | var->name;
								}
							}
						}
					}
				}
			}
			return std::nullopt;
		},
	};
};
