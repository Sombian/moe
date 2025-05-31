#pragma once

#include <cstddef>
#include <deque>
#include <variant>
#include <cstdint>
#include <utility>
#include <optional>
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
		//|--------------<chore>--------------|
		typedef std::vector<error<A, B>> report;
		//|-----------------------------------|

		struct scope
		{
			only(scope*) upper;
			many(scope*) lower;

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
			report(*)(scope*, $var&),
			report(*)(scope*, $fun&),
			report(*)(scope*, $trait&),
			report(*)(scope*, $class&),
			//|---------------|
			//| variant::stmt |
			//|---------------|
			report(*)(scope*, $if&),
			report(*)(scope*, $match&),
			report(*)(scope*, $for&),
			report(*)(scope*, $while&),
			report(*)(scope*, $break&),
			report(*)(scope*, $return&),
			report(*)(scope*, $continue&),
			//|---------------|
			//| variant::expr |
			//|---------------|
			report(*)(scope*, $unary&),
			report(*)(scope*, $binary&),
			report(*)(scope*, $literal&),
			report(*)(scope*, $symbol&),
			report(*)(scope*, $access&),
			report(*)(scope*, $group&),
			report(*)(scope*, $call&)
		>
		plugin; // scalable..?
		
		#define PLUGIN                  \
		                                \
		switch (this->stage)            \
		{                               \
			case stage::ANALYZE:        \
			{                           \
				inject(this->ptr, ast); \
			}                           \
		}                               \

		template
		<
			typename T
		>
		static constexpr
		// run plugins
		auto inject(scope* ctx, T& ast)
		{
			for (auto&& rule : RULES)
			{
				std::visit([&](auto&& fun)
				{
					typedef std::decay_t<decltype(fun)> F;

					if constexpr (std::is_invocable_v<F, scope*, T&>)
					{
						for (auto&& issue : fun(ctx, ast))
						{
							std::cout << issue << std::endl;
						}
					}
				},
				rule);
			}
		}

		static constexpr
		auto access_modifier(scope* ctx, $access& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto unsafe_lifetime(scope* ctx, $return& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto redefine_var(scope* ctx, $var& ast)
		{
			report result;

			size_t counter {0};
			// scan current scope
			for (auto&& node : ctx->define)
			{
				typedef std::pair<utf8, $var*> T;

				if (auto ptr {std::get_if<T>(&node)})
				{
					auto& [foo, bar] {*ptr};

					if (ast.y < bar->y)
					{
						break;
					}
					if (ast.name == foo)
					{
						++counter;
					}
				}
			}
			// if duplicate exists
			if (1 < counter)
			{
				std::cout << "redfine var: " << ast.name << std::endl;
			}
			return result;
		}

		static constexpr
		auto undefine_var(scope* ctx, $symbol& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto redefine_fun(scope* ctx, $fun& ast)
		{
			report result;

			size_t counter {0};
			// scan current scope
			for (auto&& node : ctx->define)
			{
				typedef std::pair<utf8, $fun*> T;

				if (auto ptr {std::get_if<T>(&node)})
				{
					auto& [name, _] {*ptr};

					if (name == ast.name)
					{
						++counter;
					}
				}
			}
			// if duplicate exists
			if (1 < counter)
			{
				std::cout << "redfine fun: " << ast.name << std::endl;
			}
			return result;
		}

		static constexpr
		auto undefine_fun(scope* ctx, $symbol& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto redefine_trait(scope* ctx, $trait& ast)
		{
			report result;

			size_t counter {0};
			// scan current scope
			for (auto&& node : ctx->define)
			{
				typedef std::pair<utf8, $trait*> T;

				if (auto ptr {std::get_if<T>(&node)})
				{
					auto& [name, _] {*ptr};

					if (name == ast.name)
					{
						++counter;
					}
				}
			}
			// if duplicate exists
			if (1 < counter)
			{
				std::cout << "redfine trait: " << ast.name << std::endl;
			}
			return result;
		}

		static constexpr
		auto undefine_trait(scope* ctx, $symbol& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto redefine_class(scope* ctx, $class& ast)
		{
			report result;

			size_t counter {0};
			// scan current scope
			for (auto&& node : ctx->define)
			{
				typedef std::pair<utf8, $class*> T;

				if (auto ptr {std::get_if<T>(&node)})
				{
					auto& [name, _] {*ptr};

					if (name == ast.name)
					{
						++counter;
					}
				}
			}
			// if duplicate exists
			if (1 < counter)
			{
				std::cout << "redfine trait: " << ast.name << std::endl;
			}
			return result;
		}

		static constexpr
		auto undefine_class(scope* ctx, $symbol& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto assign_to_this(scope* ctx, $binary& ast)
		{
			report result;
			return result;
		}

		static constexpr
		auto assign_to_const(scope* ctx, $binary& ast)
		{
			report result;
			return result;
		}

		static inline
		// define plugins
		const many(plugin) RULES
		{
			&access_modifier,
			&unsafe_lifetime,
			// var
			&redefine_var,
			&undefine_var,
			// fun
			&redefine_fun,
			&undefine_fun,
			// trait
			&redefine_trait,
			&undefine_trait,
			// class
			&redefine_class,
			&undefine_class,
			// assign
			&assign_to_this,
			&assign_to_const,
		};

	public:

		core()
		{
			auto origin {new scope};

			this->ctx = origin;
			this->ptr = origin;
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
			PLUGIN
			this->visit(ast.init);

			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src order
				.push_back(std::make_pair(ast.name, &ast));
			}
		}

		void visit($fun& ast) override
		{
			//|----<start>----|
			this->scope_start();
			//|---------------|

			PLUGIN
			this->visit(ast.args);
			this->visit(ast.body);
			
			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src order
				.push_back(std::make_pair(ast.name, &ast));
			}
			//|----<close>----|
			this->scope_close();
			//|---------------|
		}

		void visit($trait& ast) override
		{
			PLUGIN
			this->visit(ast.body);

			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src order
				.push_back(std::make_pair(ast.name, &ast));
			}
		}

		void visit($class& ast) override
		{
			//|----<start>----|
			this->scope_start();
			//|---------------|

			PLUGIN
			this->visit(ast.body);

			if (this->stage == stage::COLLECT)
			{
				this->ptr->define // as same as src order
				.push_back(std::make_pair(ast.name, &ast));
			}
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
					auto* node {new scope {this->ptr}};
					// insert new node
					this->ptr->lower.push_back(node);
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
		core impl {};

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
