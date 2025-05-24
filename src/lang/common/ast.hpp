#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <cassert>
#include <cstdint>
#include <cstddef>

#include "./token.hpp"

#include "models/str.hpp"
#include "traits/printable.hpp"
#include "traits/rule_of_5.hpp"

namespace lang
{
	struct $var;
	struct $fun;
	struct $trait;
	struct $class;
}

typedef std::variant
<
	std::unique_ptr<lang::$var>,
	std::unique_ptr<lang::$fun>,
	std::unique_ptr<lang::$trait>,
	std::unique_ptr<lang::$class>
>
decl;

namespace lang
{
	struct $if;
	struct $match;
	struct $for;
	struct $while;
	struct $break;
	struct $return;
	struct $continue;
}

typedef std::variant
<
	std::unique_ptr<lang::$if>,
	std::unique_ptr<lang::$match>,
	std::unique_ptr<lang::$for>,
	std::unique_ptr<lang::$while>,
	std::unique_ptr<lang::$break>,
	std::unique_ptr<lang::$return>,
	std::unique_ptr<lang::$continue>
>
stmt;

namespace lang
{
	struct $unary;
	struct $binary;
	struct $literal;
	struct $symbol;
	struct $access;
	struct $group;
	struct $call;
}

typedef std::variant
<
	std::unique_ptr<lang::$unary>,
	std::unique_ptr<lang::$binary>,
	std::unique_ptr<lang::$literal>,
	std::unique_ptr<lang::$symbol>,
	std::unique_ptr<lang::$access>,
	std::unique_ptr<lang::$group>,
	std::unique_ptr<lang::$call>
>
expr;

typedef std::variant
<
	decl,
	stmt,
	expr
>
node;

typedef std::vector
<
	node
>
body;

namespace
{
	//|----<only>---//---|
	template        //   |
	<typename T>    //   |
	using only = T; //   |
	//|-------------//---|
}

namespace
{
	//|----<many>---//---|
	template        //   |
	<typename T>    //   |
	using many =    //   |
	std::vector<T>; //   |
	//|-------------//---|
}

enum class data : uint8_t
{
	//|----------------|
	//| signed integer |
	//|----------------|
	I8,
	I16,
	I32,
	I64,
	//|------------------|
	//| unsigned integer |
	//|------------------|
	U8,
	U16,
	U32,
	U64,
	//|-----------------|
	//| floating points |
	//|-----------------|
	F32,
	F64,
	//|------------------|
	//| other data types |
	//|------------------|
	CODE,
	BOOL,
	WORD,
	//|-----------------|
	//| string storages |
	//|-----------------|
	UTF8,
	UTF16,
	UTF32,
};

// lhs(prefix) operator
enum class op_l : uint8_t
#define macro(K, V) K,
{
	ADD, // prefix ver.
	SUB, // prefix ver.
	operator_l(macro)
};
#undef macro

// mhs(infix) operator
enum class op_i : uint8_t
#define macro(K, V) K,
{
	operator_i(macro)
};
#undef macro

// rhs(postfix) operator
enum class op_r : uint8_t
#define macro(K, V) K,
{
	operator_r(macro)
};
#undef macro

namespace lang
{
	struct visitor
	{
		virtual void visit(const only<decl>&) = 0;
		virtual void visit(const many<decl>&) = 0;

		virtual void visit(const only<stmt>&) = 0;
		virtual void visit(const many<stmt>&) = 0;

		virtual void visit(const only<expr>&) = 0;
		virtual void visit(const many<expr>&) = 0;

		virtual void visit(const only<node>&) = 0;
		virtual void visit(const many<node>&) = 0;

		virtual void visit(const many<body>&) = 0;
		// decl
		virtual void visit($var&) = 0;
		virtual void visit($fun&) = 0;
		virtual void visit($trait&) = 0;
		virtual void visit($class&) = 0;
		// stmt
		virtual void visit($if&) = 0;
		virtual void visit($match&) = 0;
		virtual void visit($for&) = 0;
		virtual void visit($while&) = 0;
		virtual void visit($break&) = 0;
		virtual void visit($return&) = 0;
		virtual void visit($continue&) = 0;
		// expr
		virtual void visit($unary&) = 0;
		virtual void visit($binary&) = 0;
		virtual void visit($literal&) = 0;
		virtual void visit($symbol&) = 0;
		virtual void visit($access&) = 0;
		virtual void visit($group&) = 0;
		virtual void visit($call&) = 0;
	};
}

// decl
namespace lang
{
	struct $var
	{
		span span;
		only<utf8> name;
		only<utf8> type;
		only<expr> init;
		//-------------//
		bool is_const; //
		//-------------//

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $fun
	{
		span span;
		many<$var> args;
		only<utf8> name;
		only<utf8> type;
		only<body> body;
		//------------//
		bool is_pure; //
		//------------//

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $trait
	{
		span span;
		only<utf8> name;
		many<$fun> body;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $class
	{
		span span;
		only<utf8> name;
		many<$var> body;

		auto accept(visitor& impl) { impl.visit(*this); }
	};
}

// stmt
namespace lang
{
	struct $if
	{
		span span;
		many<expr> cases;
		many<body> block;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $match
	{
		span span;
		only<expr> input;
		many<expr> cases;
		many<body> block;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $for
	{
		span span;
		only<expr> setup;
		only<expr> input;
		only<expr> after;
		only<body> block;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $while
	{
		span span;
		only<expr> input;
		only<body> block;

		auto accept(visitor& impl) { impl.visit(*this); }
	};
	
	struct $break
	{
		span span;
		only<utf8> label;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $return
	{
		span span;
		only<expr> value;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $continue
	{
		span span;
		only<utf8> label;

		auto accept(visitor& impl) { impl.visit(*this); }
	};
}

// expr
namespace lang
{
	struct $unary
	{
		span span;
		only<op_l> lhs;
		only<expr> rhs;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $binary
	{
		span span;
		only<expr> lhs;
		only<op_i> mhs;
		only<expr> rhs;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $literal
	{
		span span;
		only<data> type;
		only<utf8> data;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $symbol
	{
		span span;
		only<utf8> name;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $access
	{
		span span;
		only<expr> expr;
		only<op_r> type;
		only<utf8> name;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $group
	{
		span span;
		only<expr> expr;

		auto accept(visitor& impl) { impl.visit(*this); }
	};

	struct $call
	{
		span span;
		only<expr> call;
		many<expr> args;

		auto accept(visitor& impl) { impl.visit(*this); }
	};
}

//|-----------|
//| operators |
//|-----------|

namespace opr
{
	template
	<
		typename A,
		typename B
	>
	auto is_l(const token<A, B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case atom::K:        \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			case atom::ADD:
			{
				return true;
			}
			case atom::SUB:
			{
				return true;
			}
			operator_l(macro)
			#undef macro
			default:
			{
				return false;
			}
		}
	}

	template
	<
		typename A,
		typename B
	>
	auto to_l(const token<A, B>& tkn) -> op_l
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case atom::K:        \
			{                    \
				return op_l::K;  \
			}                    \
			/*|---------------|*/\
		
			case atom::ADD:
			{
				return op_l::ADD;
			}
			case atom::SUB:
			{
				return op_l::SUB;
			}
			operator_l(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}

	template
	<
		typename A,
		typename B
	>
	auto is_i(const token<A, B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case atom::K:        \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			operator_i(macro)
			#undef macro
			default:
			{
				return false;
			}
		}
	}

	template
	<
		typename A,
		typename B
	>
	auto to_i(const token<A, B>& tkn) -> op_i
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case atom::K:        \
			{                    \
				return op_i::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_i(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}

	template
	<
		typename A,
		typename B
	>
	auto is_r(const token<A, B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case atom::K:        \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			operator_r(macro)
			#undef macro
			default:
			{
				return false;
			}
		}
	}

	template
	<
		typename A,
		typename B
	>
	auto to_r(const token<A, B>& tkn) -> op_r
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case atom::K:        \
			{                    \
				return op_r::K;  \
			}                    \
			/*|---------------|*/\
		
			operator_r(macro)
			#undef macro
			default:
			{
				assert(!!!"error");
				std::unreachable();
			}
		}
	}
}

auto operator<<(std::ostream& os, const data tp) -> std::ostream&
{
	switch (tp)
	{
		case data::I8:
		{
			return os << "i8";
		}
		case data::I16:
		{
			return os << "i16";
		}
		case data::I32:
		{
			return os << "i32";
		}
		case data::I64:
		{
			return os << "i64";
		}
		case data::U8:
		{
			return os << "u8";
		}
		case data::U16:
		{
			return os << "u16";
		}
		case data::U32:
		{
			return os << "u32";
		}
		case data::U64:
		{
			return os << "u64";
		}
		case data::F32:
		{
			return os << "f32";
		}
		case data::F64:
		{
			return os << "f64";
		}
		case data::CODE:
		{
			return os << "code";
		}
		case data::BOOL:
		{
			return os << "bool";
		}
		case data::WORD:
		{
			return os << "word";
		}
		case data::UTF8:
		{
			return os << "utf8";
		}
		case data::UTF16:
		{
			return os << "utf16";
		}
		case data::UTF32:
		{
			return os << "utf32";
		}
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
}

auto operator<<(std::ostream& os, const op_l op) -> std::ostream&
{
	switch (op)
	{
		#define macro(K, V)  \
		/*|---------------|*/\
		case op_l::K:        \
		{                    \
			return os << #K; \
		}                    \
		/*|---------------|*/\

		case op_l::ADD:
		{
			return os << "ADD";
		}
		case op_l::SUB:
		{
			return os << "SUB";
		}
		operator_l(macro)
		#undef macro
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
}

auto operator<<(std::ostream& os, const op_i op) -> std::ostream&
{
	switch (op)
	{
		#define macro(K, V)  \
		/*|---------------|*/\
		case op_i::K:        \
		{                    \
			return os << #K; \
		}                    \
		/*|---------------|*/\

		operator_i(macro)
		#undef macro
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
}

auto operator<<(std::ostream& os, const op_r op) -> std::ostream&
{
	switch (op)
	{
		#define macro(K, V)  \
		/*|---------------|*/\
		case op_r::K:        \
		{                    \
			return os << #K; \
		}                    \
		/*|---------------|*/\

		operator_r(macro)
		#undef macro
		default:
		{
			assert(!!!"error");
			std::unreachable();
		}
	}
}

//|---------------|
//| "TIRO FINALE" |
//|---------------|

struct program
{
	class printer : lang::visitor
	{
		size_t tab {0};
		std::ostream& out;

		#define START          \
		{                      \
			this->out << "\n"; \
			this->gap();       \
			this->out << "{";  \
			this->out << "\n"; \
			this->tab++;       \
		}                      \

		#define CLOSE          \
		{                      \
			this->tab--;       \
			this->gap();       \
			this->out << "}" ; \
			this->out << "\n"; \
		}                      \

		auto gap()
		{
			for (size_t i {0}; i < this->tab; ++i)
			{ /*-!-*/ this->out << "\t"; /*-!-*/ }
		}

	public:

		printer(decltype(out) out) : out {out} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		void visit(traits::printable auto& ast)
		{
			this->out << ast << "\n";
		}

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
			if (!ast.empty())
			{
				size_t count {0};

				for (auto&& node : ast)
				{
					this->visit(node);

					if (++count < ast.size())
					{
						this->gap();
						/*-<seperator>-*/
						this->out << "&";
						/*-------------*/
					}
				}
			}
			else
			{
				this->out << "[empty]\n";
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
			if (!ast.empty())
			{
				size_t count {0};

				for (auto&& node : ast)
				{
					this->visit(node);

					if (++count < ast.size())
					{
						this->gap();
						/*-<seperator>-*/
						this->out << "&";
						/*-------------*/
					}
				}
			}
			else
			{
				this->out << "[empty]\n";
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
			if (!ast.empty())
			{
				size_t count {0};

				for (auto&& node : ast)
				{
					this->visit(node);

					if (++count < ast.size())
					{
						this->gap();
						/*-<seperator>-*/
						this->out << "&";
						/*-------------*/
					}
				}
			}
			else
			{
				this->out << "[empty]\n";
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
			if (!ast.empty())
			{
				size_t count {0};

				for (auto&& node : ast)
				{
					this->visit(node);

					if (++count < ast.size())
					{
						this->gap();
						/*-<seperator>-*/
						this->out << "&";
						/*-------------*/
					}
				}
			}
			else
			{
				this->out << "[empty]\n";
			}
		}

		void visit(const many<body>& ast) override
		{
			if (!ast.empty())
			{
				size_t count {0};

				for (auto&& node : ast)
				{
					this->visit(node);

					if (++count < ast.size())
					{
						this->gap();
						/*-<seperator>-*/
						this->out << "&";
						/*-------------*/
					}
				}
			}
			else
			{
				this->out << "[empty]\n";
			}
		}
		
		#define VISIT($T)                      \
		                                       \
		void visit(many<$T>& list)             \
		{                                      \
			if (!list.empty())                 \
			{                                  \
				size_t count {0};              \
				                               \
				for (auto&& ast : list)        \
				{                              \
					this->visit(ast);          \
					                           \
					if (++count < list.size()) \
					{                          \
						this->gap();           \
						/*-<seperator>-*/      \
						this->out << "&";      \
						/*-------------*/      \
					}                          \
				}                              \
			}                                  \
			else                               \
			{                                  \
				this->out << "[empty]\n";      \
			}                                  \
		}                                      \

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit(lang::$var& ast) override
		{
			START
			this->gap(); this->out << "[var]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(ast.name);
			this->gap(); this->out << "type" << ": "; this->visit(ast.type);
			this->gap(); this->out << "init" << ": "; this->visit(ast.init);
			CLOSE
		}
		VISIT(lang::$var)

		void visit(lang::$fun& ast) override
		{

			START
			this->gap(); this->out << "[fun]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(ast.name);
			this->gap(); this->out << "type" << ": "; this->visit(ast.type);
			this->gap(); this->out << "body" << ": "; this->visit(ast.body);
			CLOSE
		}
		VISIT(lang::$fun)

		void visit(lang::$trait& ast) override
		{
			START
			this->gap(); this->out << "[trait]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(ast.name);
			this->gap(); this->out << "body" << ": "; this->visit(ast.body);
			CLOSE
		}
		VISIT(lang::$trait)

		void visit(lang::$class& ast) override
		{
			START
			this->gap(); this->out << "[class]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(ast.name);
			this->gap(); this->out << "body" << ": "; this->visit(ast.body);
			CLOSE
		}
		VISIT(lang::$class)

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit(lang::$if& ast) override
		{
			START
			this->gap(); this->out << "[if]" << "\n";
			this->gap(); this->out << "block" << ": "; this->visit(ast.block);
			this->gap(); this->out << "cases" << ": "; this->visit(ast.cases);
			CLOSE
		}
		VISIT(lang::$if)

		void visit(lang::$match& ast) override
		{
			START
			this->gap(); this->out << "[match]" << "\n";
			this->gap(); this->out << "input" << ": "; this->visit(ast.input);
			this->gap(); this->out << "block" << ": "; this->visit(ast.block);
			this->gap(); this->out << "cases" << ": "; this->visit(ast.cases);
			CLOSE
		}
		VISIT(lang::$match)

		void visit(lang::$for& ast) override
		{
			START
			this->gap(); this->out << "[for]" << "\n";
			this->gap(); this->out << "setup" << ": "; this->visit(ast.setup);
			this->gap(); this->out << "input" << ": "; this->visit(ast.input);
			this->gap(); this->out << "after" << ": "; this->visit(ast.after);
			this->gap(); this->out << "block" << ": "; this->visit(ast.block);
			CLOSE
		}
		VISIT(lang::$for)

		void visit(lang::$while& ast) override
		{
			START
			this->gap(); this->out << "[while]" << "\n";
			this->gap(); this->out << "input" << ": "; this->visit(ast.input);
			this->gap(); this->out << "block" << ": "; this->visit(ast.block);
			CLOSE
		}
		VISIT(lang::$while)

		void visit(lang::$break& ast) override
		{
			START
			this->gap(); this->out << "[break]" << "\n";
			this->gap(); this->out << "label" << ": "; this->visit(ast.label);
			CLOSE
		}
		VISIT(lang::$break)

		void visit(lang::$return& ast) override
		{
			START
			this->gap(); this->out << "[return]" << "\n";
			this->gap(); this->out << "value" << ": "; this->visit(ast.value);
			CLOSE
		}
		VISIT(lang::$return)

		void visit(lang::$continue& ast) override
		{
			START
			this->gap(); this->out << "[continue]" << "\n";
			this->gap(); this->out << "label" << ": "; this->visit(ast.label);
			CLOSE
		}
		VISIT(lang::$continue)

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit(lang::$unary& ast) override
		{
			START
			this->gap(); this->out << "[unary]" << "\n";
			this->gap(); this->out << "lhs" << ": "; this->visit(ast.lhs);
			this->gap(); this->out << "rhs" << ": "; this->visit(ast.rhs);
			CLOSE
		}
		VISIT(lang::$unary)

		void visit(lang::$binary& ast) override
		{
			START
			this->gap(); this->out << "[binary]" << "\n";
			this->gap(); this->out << "lhs" << ": "; this->visit(ast.lhs);
			this->gap(); this->out << "mhs" << ": "; this->visit(ast.mhs);
			this->gap(); this->out << "rhs" << ": "; this->visit(ast.rhs);
			CLOSE
		}
		VISIT(lang::$binary)

		void visit(lang::$literal& ast) override
		{
			START
			this->gap(); this->out << "[literal]" << "\n";
			this->gap(); this->out << "type" << ": "; this->visit(ast.type);
			this->gap(); this->out << "data" << ": "; this->visit(ast.data);
			CLOSE
		}
		VISIT(lang::$literal)

		void visit(lang::$symbol& ast) override
		{

			START
			this->gap(); this->out << "[symbol]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(ast.name);
			CLOSE
		}
		VISIT(lang::$symbol)

		void visit(lang::$access& data) override
		{
			START
			this->gap(); this->out << "[access]" << "\n";
			this->gap(); this->out << "expr" << ": "; this->visit(data.expr);
			this->gap(); this->out << "type" << ": "; this->visit(data.type);
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			CLOSE
		}
		VISIT(lang::$access)

		void visit(lang::$group& data) override
		{
			START
			this->gap(); this->out << "[group]" << "\n";
			this->gap(); this->out << "expr" << ": "; this->visit(data.expr);
			CLOSE
		}
		VISIT(lang::$group)

		void visit(lang::$call& data) override
		{
			START
			this->gap(); this->out << "[call]" << "\n";
			this->gap(); this->out << "call" << ": "; this->visit(data.call);
			this->gap(); this->out << "args" << ": "; this->visit(data.args);
			CLOSE
		}
		VISIT(lang::$call)

		#undef START
		#undef CLOSE
		#undef VISIT
	};

public:

	many<node> ast;

	program() = default;

	COPY_CONSTRUCTOR(program) = delete;
	MOVE_CONSTRUCTOR(program) = default;

	COPY_ASSIGNMENT(program) = delete;
	MOVE_ASSIGNMENT(program) = default;

	//|-----------------|
	//| member function |
	//|-----------------|

	auto print() const
	{
		printer impl {std::cout};

		for (auto&& node : this->ast)
		{
			std::visit([&](auto&& arg)
			{
				impl.visit(arg);
			},
			node);
		}
	}
};
