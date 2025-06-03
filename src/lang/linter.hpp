#pragma once

#include <deque>
#include <memory>
#include <variant>
#include <cstdint>
#include <utility>
#include <optional>
#include <concepts>
#include <type_traits>

#include "models/str.hpp"

#include "lang/parser.hpp"

#include "./common/ast.hpp"
#include "./common/error.hpp"

template
<
	type::string A,
	type::string B
>
class linter
{
	parser<A, B>* parser;

	#define only(T) T /* sugar */
	#define many(T) std::deque<T>

	//|----------<buffer>----------|
	decltype(parser->pull()) buffer;
	//|----------------------------|

	struct core : public lang::visitor
	{
		//|---------------<chore>---------------|
		typedef std::optional<error<A, B>> report;
		//|-------------------------------------|

		struct scope
		{
			only(scope*) upper {nullptr};
			many(scope*) lower; // empty

			std::vector<std::variant
			<
				std::pair<utf8, $var*>,
				std::pair<utf8, $fun*>,
				std::pair<utf8, $trait*>,
				std::pair<utf8, $class*>
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
		//|-----<file>-----|
		fs::file<A, B>* src;
		//|----------------|
		scope* ctx;
		scope* ptr;

		enum class stage : uint8_t
		{
			COLLECT,
			ANALYZE,
		};
		stage stage {stage::COLLECT};

		typedef std::variant
		<
			//|---------------|
			//| variant::decl |
			//|---------------|
			report(*)(fs::file<A, B>*, scope*, $var&),
			report(*)(fs::file<A, B>*, scope*, $fun&),
			report(*)(fs::file<A, B>*, scope*, $trait&),
			report(*)(fs::file<A, B>*, scope*, $class&),
			//|---------------|
			//| variant::stmt |
			//|---------------|
			report(*)(fs::file<A, B>*, scope*, $if&),
			report(*)(fs::file<A, B>*, scope*, $match&),
			report(*)(fs::file<A, B>*, scope*, $for&),
			report(*)(fs::file<A, B>*, scope*, $while&),
			report(*)(fs::file<A, B>*, scope*, $break&),
			report(*)(fs::file<A, B>*, scope*, $return&),
			report(*)(fs::file<A, B>*, scope*, $continue&),
			//|---------------|
			//| variant::expr |
			//|---------------|
			report(*)(fs::file<A, B>*, scope*, $unary&),
			report(*)(fs::file<A, B>*, scope*, $binary&),
			report(*)(fs::file<A, B>*, scope*, $literal&),
			report(*)(fs::file<A, B>*, scope*, $symbol&),
			report(*)(fs::file<A, B>*, scope*, $access&),
			report(*)(fs::file<A, B>*, scope*, $group&),
			report(*)(fs::file<A, B>*, scope*, $call&)
		>
		plugin; // scalable..?
		
		#define PLUGIN                         \
		                                       \
		if (this->stage == stage::ANALYZE)     \
		{                                      \
			inject(this->src, this->ptr, ast); \
		}                                      \

		template
		<
			typename T
		>
		static constexpr
		// run plugins
		auto inject(fs::file<A, B>* src, scope* ctx, T& ast)
		{
			typedef decltype(src) X;
			typedef decltype(ctx) Y;

			for (auto&& rule : RULES)
			{
				std::visit([&](auto&& fun)
				{
					typedef std::decay_t<decltype(fun)> F;

					if constexpr (std::is_invocable_v<F, X, Y, T&>)
					{
						if (auto error {fun(src, ctx, ast)})
						{
							std::cout << *error << '\n';
						}
					}
				},
				rule);
			}
		}

		#define E(value) error<A, B> \
		{                            \
			ast.x, ast.y, src, value \
		}                            \

		static constexpr
		auto access_modifier(fs::file<A, B>* src, scope* ctx, $access& ast) -> report
		{
			return std::nullopt;
		}

		static constexpr
		auto unsafe_lifetime(fs::file<A, B>* src, scope* ctx, $return& ast) -> report
		{
			return std::nullopt;
		}

		static constexpr
		auto redefine_var(fs::file<A, B>* src, scope* ctx, $var& ast) -> report
		{
			if (auto ptr {resolve<$var*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
					       &&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return E
			(
				u8"variable definition '%s' already exists"
				|
				ast.name // YES WE CAN! string interpolation
			);
		}

		static constexpr
		auto redefine_fun(fs::file<A, B>* src, scope* ctx, $fun& ast) -> report
		{
			if (auto ptr {resolve<$fun*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
					       &&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return E
			(
				u8"function definition '%s' already exists"
				|
				ast.name // YES WE CAN! string interpolation
			);
		}

		static constexpr
		auto redefine_trait(fs::file<A, B>* src, scope* ctx, $trait& ast) -> report
		{
			if (auto ptr {resolve<$trait*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
					       &&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return E
			(
				u8"trait definition '%s' already exists"
				|
				ast.name // YES WE CAN! string interpolation
			);
		}

		static constexpr
		auto redefine_class(fs::file<A, B>* src, scope* ctx, $class& ast) -> report
		{
			if (auto ptr {resolve<$class*>(ctx, ast.name)})
			{
				if (ptr->x == ast.x
					       &&
					ptr->y == ast.y)
				{
					return std::nullopt;
				}
			}
			return E
			(
				u8"class definition '%s' already exists"
				|
				ast.name // YES WE CAN! string interpolation
			);
		}

		static constexpr
		auto assign_to_this(fs::file<A, B>* src, scope* ctx, $binary& ast) -> report
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
		}

		static constexpr
		auto assign_to_let$(fs::file<A, B>* src, scope* ctx, $binary& ast) -> report
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
							auto var {resolve<$var*>(_ctx, (*ptr)->name)};

							// checking '<=' since src position does matter
							if (var != nullptr && var->is_const && *var <= ast)
							{
								return E
								(
									u8"cannot assign to constant variable '%s'"
									|
									var->name // YES WE CAN! string interpolation
								);
							}
						}
					}
				}
			}
			return std::nullopt;
		}

		static constexpr
		auto assign_to_temp(fs::file<A, B>* src, scope* ctx, $binary& ast) -> report
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
		}

		static constexpr
		auto unknown_symbol(fs::file<A, B>* src, scope* ctx, $symbol& ast) -> report
		{
			for (auto _ctx {ctx}; _ctx; _ctx = _ctx->upper)
			{
				if (auto ptr {resolve<$var*>(_ctx, ast.name)})
				{
					return std::nullopt;
				}
				if (auto ptr {resolve<$fun*>(_ctx, ast.name)})
				{
					return std::nullopt;
				}
				if (auto ptr {resolve<$trait*>(_ctx, ast.name)})
				{
					return std::nullopt;
				}
				if (auto ptr {resolve<$class*>(_ctx, ast.name)})
				{
					return std::nullopt;
				}
			}
			return E
			(
				u8"cannot find definition for symbol '%s'"
				|
				ast.name // YES WE CAN! string interpolation
			);
		}

		static inline
		// define plugins
		const many(plugin) RULES
		{
			// safety
			&access_modifier,
			&unsafe_lifetime,
			// redefine
			&redefine_var,
			&redefine_fun,
			&redefine_trait,
			&redefine_class,
			// assignment
			&assign_to_this,
			&assign_to_let$,
			&assign_to_temp,
			// miscelinouus
			&unknown_symbol,
		};

		#undef E

	public:

		core(decltype(src) src) : src {src}
		{
			auto root {new scope};

			this->ctx = root;
			this->ptr = root;
		}

		~core()
		{
			delete this->ctx;
		}

		//|-----------------|
		//| member function |
		//|-----------------|

		void visit(const only(decl)& ast) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(decl)& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only(stmt)& ast) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(stmt)& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only(expr)& ast) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(expr)& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only(node)& ast) override
		{
			std::visit([&](auto&& arg)
			{
				this->visit(arg);
			},
			ast);
		}

		void visit(const many(node)& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const many(body)& ast) override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		template
		<
			typename T
		>
		requires
		(
			std::is_same_v<T, many($var)>
			||
			std::is_same_v<T, many($fun)>
		)
		void visit(T& _)
		{
			for (auto&& node : _)
			{ 
				this->visit(node);
			}
		}

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit($var& ast) override
		{
			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src file order
				.emplace_back(std::make_pair(ast.name, &ast));
			}
			PLUGIN
			this->visit(ast.init);
		}

		void visit($fun& ast) override
		{
			//|----<start>----|
			this->scope_start();
			//|---------------|

			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src file order
				.emplace_back(std::make_pair(ast.name, &ast));
			}
			PLUGIN
			this->visit(ast.args);
			this->visit(ast.body);
			
			//|----<close>----|
			this->scope_close();
			//|---------------|
		}

		void visit($trait& ast) override
		{
			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src file order
				.emplace_back(std::make_pair(ast.name, &ast));
			}
			PLUGIN
			this->visit(ast.body);
		}

		void visit($class& ast) override
		{
			//|----<start>----|
			this->scope_start();
			//|---------------|
			
			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src file order
				.emplace_back(std::make_pair(ast.name, &ast));
			}
			PLUGIN
			this->visit(ast.body);

			//|----<close>----|
			this->scope_close();
			//|---------------|
		}

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit($if& ast) override
		{
			PLUGIN
			this->visit(ast.cases);
			this->visit(ast.block);
		}

		void visit($match& ast) override
		{
			PLUGIN
			this->visit(ast.input);
			this->visit(ast.cases);
			this->visit(ast.block);
		}

		void visit($for& ast) override
		{ 
			PLUGIN
			this->visit(ast.setup);
			this->visit(ast.input);
			this->visit(ast.after);
			this->visit(ast.block);
		}

		void visit($while& ast) override
		{
			PLUGIN
			this->visit(ast.input);
			this->visit(ast.block);
		}

		void visit($break& ast) override
		{
			PLUGIN
		}

		void visit($return& ast) override
		{
			PLUGIN
			this->visit(ast.value);
		}

		void visit($continue& ast) override
		{
			PLUGIN
		}

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit($unary& ast) override
		{
			PLUGIN
			this->visit(ast.rhs);
		}

		void visit($binary& ast) override 
		{
			PLUGIN
			this->visit(ast.lhs);
			this->visit(ast.rhs);
		}

		void visit($literal& ast) override
		{
			PLUGIN
		}

		void visit($symbol& ast) override
		{
			PLUGIN
		}

		void visit($access& ast) override
		{
			PLUGIN
		}

		void visit($group& ast) override
		{
			PLUGIN
			this->visit(ast.expr);
		}

		void visit($call& ast) override
		{
			PLUGIN
			this->visit(ast.args);
			this->visit(ast.call);
		}

	private:

		auto scope_start()
		{
			switch (this->stage)
			{
				case stage::COLLECT:
				{
					// create new node
					auto node {new scope {this->ptr}};
					// insert new node
					this->ptr->lower.emplace_back(node);
					// update the ptr
					this->ptr = this->ptr->lower.back();
					break;
				}
				case stage::ANALYZE:
				{
					// climb down
					this->ptr = this->ptr->lower[0];
					break;
				}
			}
		}

		auto scope_close()
		{
			switch (this->stage)
			{
				case stage::COLLECT:
				{
					// climb up
					this->ptr = this->ptr->upper;
					break;
				}
				case stage::ANALYZE:
				{
					// climb up
					this->ptr = this->ptr->upper;
					// drop ctx
					this->ptr->lower.pop_front();
					break;
				}
			}
		}

		template
		<
			typename T
		>
		requires
		(
			std::is_pointer_v<T>
		)
		static constexpr
		auto resolve(scope* ctx, utf8& name) -> T
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
	};

	#undef only
	#undef many

public:

	linter
	(
		decltype(parser) parser
	)
	: parser {parser}, buffer {parser->pull()} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	operator fs::file<A, B>*()
	{
		return *this->parser;
	}

	auto pull() -> std::optional<program>&
	{
		core impl {*this};

		if (auto&& exe {this->buffer})
		{
			// step 1. collect decl
			impl.stage = core::stage::COLLECT;
			
			for (auto&& node : exe->ast)
			{
				std::visit([&](auto&& arg)
				{
					impl.visit(arg);
				},
				node);
			}
			// step 2. collect issue
			impl.stage = core::stage::ANALYZE;

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
