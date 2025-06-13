#pragma once

#include <deque>
#include <vector>
#include <variant>
#include <utility>
#include <optional>
#include <type_traits>

#include "./common/ast.hpp"

#include "lang/2_parser.hpp"

template
<
	type::string A,
	type::string B
>
class linter
{
	friend class core;

	parser<A, B>* parser;

	#define only(T) T /* sugar */
	#define many(T) std::deque<T>

	struct scope
	{
		only(scope*) upper {nullptr};
		many(scope*) lower {/*^-^*/};

		std::vector<std::variant
		<
			std::pair<utf8, $var*>,
			std::pair<utf8, $fun*>,
			std::pair<utf8, $trait*>,
			std::pair<utf8, $model*>
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

	#undef only
	#undef many

	scope* ctx {new scope {nullptr}};

	//|----------<buffer>----------|
	decltype(parser->pull()) buffer;
	//|----------------------------|

	template
	<
		typename T
	>
	static constexpr auto lookup(scope* ctx, utf8& name) -> T
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
	static constexpr auto inject(scope* ctx, T& ast) -> std::vector<utf8>
	{
		std::vector<utf8> report;

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
		class core : public lang::reflect
		{
			#define START                            \
			{                                        \
				scope* node {new scope {this->ctx}}; \
				this->ctx->lower.emplace_back(node); \
				this->ctx = this->ctx->lower.back(); \
			}                                        \
			
			#define CLOSE                     \
			{                                 \
				this->ctx = this->ctx->upper; \
			}                                 \
			
			//|----<safe----|
			linter<A, B>* src;
			//|-------------|
			mutable scope* ctx;
			
		public:

			core // visitor pattern impl
			(
				decltype(src) src
			)
			: src {src}, ctx {src->ctx} {}
		
			//|-------------|
			//| inheritance |
			//|-------------|

			using reflect::visit;
			
			//|---------------|
			//| variant::decl |
			//|---------------|

			inline constexpr void visit($fun& ast) override
			{
				START

				// insert to the ctx
				{
					this->ctx->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.args);
				this->visit(ast.body);

				CLOSE
			}

			inline constexpr void visit($var& ast) override
			{
				// insert to the ctx
				{
					this->ctx->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.init);
			}

			inline constexpr void visit($model& ast) override
			{
				START

				// insert to the ctx
				{
					this->ctx->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.body);

				CLOSE
			}

			inline constexpr void visit($trait& ast) override
			{
				// insert to the ctx
				{
					this->ctx->define /* as same as the def idx */
					.emplace_back(std::make_pair(ast.name, &ast));
				}
				this->visit(ast.body);
			}

			#undef START
			#undef CLOSE
		}
		impl {this};

		this->buffer.visit(impl);
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

	inline constexpr auto pull() -> program<A, B>
	{
		class core : public lang::reflect
		{	
			#define CHECK                                \
			{                                            \
				for (auto& msg : inject(this->ctx, ast)) \
				{                                        \
					this->src->buffer.error.emplace_back \
					(                                    \
						ast.x, ast.y, *this->src, msg    \
					);                                   \
				}                                        \
			}                                            \

			#define START                        \
			{                                    \
				this->ctx = this->ctx->lower[0]; \
			}                                    \
			
			#define CLOSE                     \
			{                                 \
				this->ctx = this->ctx->upper; \
				this->ctx->lower.pop_front(); \
			}                                 \
			
			//|----<safe----|
			linter<A, B>* src;
			//|-------------|
			mutable scope* ctx;

		public:

			core // visitor pattern impl
			(
				decltype(src) src
			)
			: src {src}, ctx {src->ctx} {}
		
			//|-------------|
			//| inheritance |
			//|-------------|

			using reflect::visit;

			//|---------------|
			//| variant::decl |
			//|---------------|

			inline constexpr void visit($fun& ast) override
			{
				START

				CHECK
				this->visit(ast.args);
				this->visit(ast.body);
				
				CLOSE
			}

			inline constexpr void visit($var& ast) override
			{
				CHECK
				this->visit(ast.init);
			}

			inline constexpr void visit($model& ast) override
			{
				START

				CHECK
				this->visit(ast.body);

				CLOSE
			}

			inline constexpr void visit($trait& ast) override
			{
				CHECK
				this->visit(ast.body);
			}

			#undef START
			#undef CLOSE

			//|---------------|
			//| variant::stmt |
			//|---------------|

			inline constexpr void visit($if& ast) override
			{
				CHECK
				this->visit(ast.cases);
				this->visit(ast.block);
			}

			inline constexpr void visit($for& ast) override
			{ 
				CHECK
				this->visit(ast.setup);
				this->visit(ast.input);
				this->visit(ast.after);
				this->visit(ast.block);
			}

			inline constexpr void visit($match& ast) override
			{
				CHECK
				this->visit(ast.input);
				this->visit(ast.cases);
				this->visit(ast.block);
			}

			inline constexpr void visit($while& ast) override
			{
				CHECK
				this->visit(ast.input);
				this->visit(ast.block);
			}

			inline constexpr void visit($break& ast) override
			{
				CHECK
			}

			inline constexpr void visit($return& ast) override
			{
				CHECK
				this->visit(ast.value);
			}

			inline constexpr void visit($continue& ast) override
			{
				CHECK
			}

			//|---------------|
			//| variant::expr |
			//|---------------|

			inline constexpr void visit($unary& ast) override
			{
				CHECK
				this->visit(ast.rhs);
			}

			inline constexpr void visit($binary& ast) override
			{
				CHECK
				this->visit(ast.lhs);
				this->visit(ast.rhs);
			}

			inline constexpr void visit($literal& ast) override
			{
				CHECK
			}

			inline constexpr void visit($symbol& ast) override
			{
				CHECK
			}

			inline constexpr void visit($access& ast) override
			{
				CHECK
			}

			inline constexpr void visit($group& ast) override
			{
				CHECK
				this->visit(ast.expr);
			}

			inline constexpr void visit($call& ast) override
			{
				CHECK
				this->visit(ast.args);
				this->visit(ast.call);
			}

			#undef CHECK
		}
		impl {this};
		
		this->buffer.visit(impl);

		return std::move(this->buffer);
	}

private:

	static inline
	// define plugins
	const std::variant
	<
		//|---------------|
		//| variant::decl |
		//|---------------|
		std::optional<utf8>(*)(scope*, $fun&),
		std::optional<utf8>(*)(scope*, $var&),
		std::optional<utf8>(*)(scope*, $model&),
		std::optional<utf8>(*)(scope*, $trait&),
		//|---------------|
		//| variant::stmt |
		//|---------------|
		std::optional<utf8>(*)(scope*, $if&),
		std::optional<utf8>(*)(scope*, $for&),
		std::optional<utf8>(*)(scope*, $match&),
		std::optional<utf8>(*)(scope*, $while&),
		std::optional<utf8>(*)(scope*, $break&),
		std::optional<utf8>(*)(scope*, $return&),
		std::optional<utf8>(*)(scope*, $continue&),
		//|---------------|
		//| variant::expr |
		//|---------------|
		std::optional<utf8>(*)(scope*, $unary&),
		std::optional<utf8>(*)(scope*, $binary&),
		std::optional<utf8>(*)(scope*, $literal&),
		std::optional<utf8>(*)(scope*, $symbol&),
		std::optional<utf8>(*)(scope*, $access&),
		std::optional<utf8>(*)(scope*, $group&),
		std::optional<utf8>(*)(scope*, $call&)
	>
	RULES[]
	{
		[](scope* ctx, $symbol& ast) -> std::optional<utf8>
		{
			for (auto _ctx {ctx}; _ctx; _ctx = _ctx->upper)
			{
				if (auto ptr {lookup<$fun*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
				if (auto ptr {lookup<$var*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
				if (auto ptr {lookup<$model*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
				if (auto ptr {lookup<$trait*>(_ctx, ast.name)})
				{
					if (*ptr < ast) { return std::nullopt; }
				}
			}
			return u8"[linter] cannot find definition for symbol '%s'" | ast.name;
		},
		[](scope* ctx, $fun& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<$fun*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"[linter] function definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, $var& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<$var*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"[linter] variable definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, $model& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<$model*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"[linter] class definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, $trait& ast) -> std::optional<utf8>
		{
			if (auto ptr {lookup<$trait*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
							&&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return u8"[linter] trait definition '%s' already exists" | ast.name;
		},
		[](scope* ctx, $binary& ast) -> std::optional<utf8>
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
		[](scope* ctx, $binary& ast) -> std::optional<utf8>
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
		[](scope* ctx, $binary& ast) -> std::optional<utf8>
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
							if (auto var {lookup<$var*>(_ctx, (*ptr)->name)})
							{
								if (*var < ast)
								{
									if (!var->is_const)
									{
										return std::nullopt;
									}
									return u8"[linter] cannot assign to constant variable '%s'" | var->name;
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
