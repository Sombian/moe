#pragma once

#include <deque>
#include <vector>
#include <memory>
#include <variant>
#include <cassert>
#include <cstdint>

#include "./token.hpp"
#include "./error.hpp"

#include "models/str.hpp"

#include "traits/rule_of_5.hpp"

// lhs(prefix) operator
enum class op_l : uint8_t
#define macro(K, V) K,
{
	ADD, // prefix ver.
	SUB, // prefix ver.
	operator_l(macro)
};
#undef macro

// infix operator
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

enum class data : uint8_t
{
	//|----------------|
	//| signed integer |
	//|----------------|
	I8,
	I16,
	I32,
	I64,
	//|-----------------|
	//| floating points |
	//|-----------------|
	F32,
	F64,
	//|------------------|
	//| unsigned integer |
	//|------------------|
	U8,
	U16,
	U32,
	U64,
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
			assert(!"[ERROR]");
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
			assert(!"[ERROR]");
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
			assert(!"[ERROR]");
			std::unreachable();
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
			assert(!"[ERROR]");
			std::unreachable();
		}
	}
}

namespace lang
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
				assert(!"[ERROR]");
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
				assert(!"[ERROR]");
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
				assert(!"[ERROR]");
				std::unreachable();
			}
		}
	}
}

typedef std::variant
<
	std::unique_ptr<struct $var>,
	std::unique_ptr<struct $fun>,
	std::unique_ptr<struct $class>,
	std::unique_ptr<struct $trait>
>
decl;

typedef std::unique_ptr<struct $var> var_t;
typedef std::unique_ptr<struct $fun> fun_t;
typedef std::unique_ptr<struct $trait> trait_t;
typedef std::unique_ptr<struct $class> class_t;

typedef std::variant
<
	std::unique_ptr<struct $if>,
	std::unique_ptr<struct $for>,
	std::unique_ptr<struct $match>,
	std::unique_ptr<struct $while>,
	std::unique_ptr<struct $break>,
	std::unique_ptr<struct $return>,
	std::unique_ptr<struct $continue>
>
stmt;

typedef std::unique_ptr<$if> if_t;
typedef std::unique_ptr<$for> for_t;
typedef std::unique_ptr<$match> match_t;
typedef std::unique_ptr<$while> while_t;
typedef std::unique_ptr<$break> break_t;
typedef std::unique_ptr<$return> return_t;
typedef std::unique_ptr<$continue> continue_t;

typedef std::variant
<
	std::unique_ptr<struct $unary>,
	std::unique_ptr<struct $binary>,
	std::unique_ptr<struct $literal>,
	std::unique_ptr<struct $symbol>,
	std::unique_ptr<struct $access>,
	std::unique_ptr<struct $group>,
	std::unique_ptr<struct $call>
>
expr;

typedef std::unique_ptr<$unary> unary_t;
typedef std::unique_ptr<$binary> binary_t;
typedef std::unique_ptr<$literal> literal_t;
typedef std::unique_ptr<$symbol> symbol_t;
typedef std::unique_ptr<$access> access_t;
typedef std::unique_ptr<$group> group_t;
typedef std::unique_ptr<$call> call_t;

typedef std::variant
<
	decl,
	stmt,
	expr
>
node;

typedef std::deque
<
	node
>
body;

#define only(T) T /* sugar */
#define many(T) std::deque<T>

namespace lang
{
	struct visitor
	{
		virtual void visit(const only(decl)&) const = 0;
		virtual void visit(const many(decl)&) const = 0;

		virtual void visit(const only(stmt)&) const = 0;
		virtual void visit(const many(stmt)&) const = 0;

		virtual void visit(const only(expr)&) const = 0;
		virtual void visit(const many(expr)&) const = 0;

		virtual void visit(const only(node)&) const = 0;
		virtual void visit(const many(node)&) const = 0;

		virtual void visit(const many(body)&) const = 0;

		//|---------------|
		//| variant::decl |
		//|---------------|

		virtual void visit(const $var&) const = 0;
		virtual void visit(const $fun&) const = 0;
		virtual void visit(const $trait&) const = 0;
		virtual void visit(const $class&) const = 0;

		//|---------------|
		//| variant::stmt |
		//|---------------|

		virtual void visit(const $if&) const = 0;
		virtual void visit(const $for&) const = 0;
		virtual void visit(const $match&) const = 0;
		virtual void visit(const $while&) const = 0;
		virtual void visit(const $break&) const = 0;
		virtual void visit(const $return&) const = 0;
		virtual void visit(const $continue&) const = 0;

		//|---------------|
		//| variant::expr |
		//|---------------|

		virtual void visit(const $unary&) const = 0;
		virtual void visit(const $binary&) const = 0;
		virtual void visit(const $literal&) const = 0;
		virtual void visit(const $symbol&) const = 0;
		virtual void visit(const $access&) const = 0;
		virtual void visit(const $group&) const = 0;
		virtual void visit(const $call&) const = 0;
	};
}

//|---------------|
//| variant::decl |
//|---------------|

struct $var : public span
{
	only(utf8) name;
	only(utf8) type;
	only(expr) init;
	//-------------//
	bool is_const; //
	//-------------//

	$var() = default;

	COPY_CONSTRUCTOR($var) = delete;
	MOVE_CONSTRUCTOR($var) = default;

	COPY_ASSIGNMENT($var) = delete;
	MOVE_ASSIGNMENT($var) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $fun : public span
{
	only(utf8) name;
	many($var) args;
	only(utf8) type;
	only(body) body;
	//------------//
	bool is_pure; //
	//------------//

	$fun() = default;

	COPY_CONSTRUCTOR($fun) = delete;
	MOVE_CONSTRUCTOR($fun) = default;

	COPY_ASSIGNMENT($fun) = delete;
	MOVE_ASSIGNMENT($fun) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $trait : public span
{
	only(utf8) name;
	many($fun) body;

	$trait() = default;

	COPY_CONSTRUCTOR($trait) = delete;
	MOVE_CONSTRUCTOR($trait) = default;

	COPY_ASSIGNMENT($trait) = delete;
	MOVE_ASSIGNMENT($trait) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $class : public span
{
	only(utf8) name;
	many($var) body;

	$class() = default;

	COPY_CONSTRUCTOR($class) = delete;
	MOVE_CONSTRUCTOR($class) = default;

	COPY_ASSIGNMENT($class) = delete;
	MOVE_ASSIGNMENT($class) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

//|---------------|
//| variant::stmt |
//|---------------|

struct $if : public span
{
	many(expr) cases;
	many(body) block;

	$if() = default;

	COPY_CONSTRUCTOR($if) = delete;
	MOVE_CONSTRUCTOR($if) = default;

	COPY_ASSIGNMENT($if) = delete;
	MOVE_ASSIGNMENT($if) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $match : public span
{
	only(expr) input;
	many(expr) cases;
	many(body) block;

	$match() = default;

	COPY_CONSTRUCTOR($match) = delete;
	MOVE_CONSTRUCTOR($match) = default;

	COPY_ASSIGNMENT($match) = delete;
	MOVE_ASSIGNMENT($match) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $for : public span
{
	only(expr) setup;
	only(expr) input;
	only(expr) after;
	only(body) block;

	$for() = default;

	COPY_CONSTRUCTOR($for) = delete;
	MOVE_CONSTRUCTOR($for) = default;

	COPY_ASSIGNMENT($for) = delete;
	MOVE_ASSIGNMENT($for) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $while : public span
{
	only(expr) input;
	only(body) block;

	$while() = default;

	COPY_CONSTRUCTOR($while) = delete;
	MOVE_CONSTRUCTOR($while) = default;

	COPY_ASSIGNMENT($while) = delete;
	MOVE_ASSIGNMENT($while) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $break : public span
{
	only(utf8) label;

	$break() = default;

	COPY_CONSTRUCTOR($break) = delete;
	MOVE_CONSTRUCTOR($break) = default;

	COPY_ASSIGNMENT($break) = delete;
	MOVE_ASSIGNMENT($break) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $return : public span
{
	only(expr) value;

	$return() = default;

	COPY_CONSTRUCTOR($return) = delete;
	MOVE_CONSTRUCTOR($return) = default;

	COPY_ASSIGNMENT($return) = delete;
	MOVE_ASSIGNMENT($return) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $continue : public span
{
	only(utf8) label;

	$continue() = default;

	COPY_CONSTRUCTOR($continue) = delete;
	MOVE_CONSTRUCTOR($continue) = default;

	COPY_ASSIGNMENT($continue) = delete;
	MOVE_ASSIGNMENT($continue) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

//|---------------|
//| variant::expr |
//|---------------|

struct $unary : public span
{
	only(op_l) op;
	only(expr) rhs;

	$unary() = default;

	COPY_CONSTRUCTOR($unary) = delete;
	MOVE_CONSTRUCTOR($unary) = default;

	COPY_ASSIGNMENT($unary) = delete;
	MOVE_ASSIGNMENT($unary) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $binary : public span
{
	only(op_i) op;
	only(expr) lhs;
	only(expr) rhs;

	$binary() = default;

	COPY_CONSTRUCTOR($binary) = delete;
	MOVE_CONSTRUCTOR($binary) = default;

	COPY_ASSIGNMENT($binary) = delete;
	MOVE_ASSIGNMENT($binary) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $literal : public span
{
	only(data) type;
	only(utf8) data;

	$literal() = default;

	COPY_CONSTRUCTOR($literal) = delete;
	MOVE_CONSTRUCTOR($literal) = default;

	COPY_ASSIGNMENT($literal) = delete;
	MOVE_ASSIGNMENT($literal) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $symbol : public span
{
	only(utf8) name;

	$symbol() = default;

	COPY_CONSTRUCTOR($symbol) = delete;
	MOVE_CONSTRUCTOR($symbol) = default;

	COPY_ASSIGNMENT($symbol) = delete;
	MOVE_ASSIGNMENT($symbol) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $access : public span
{
	only(expr) expr;
	only(op_r) type;
	only(utf8) name;

	$access() = default;

	COPY_CONSTRUCTOR($access) = delete;
	MOVE_CONSTRUCTOR($access) = default;

	COPY_ASSIGNMENT($access) = delete;
	MOVE_ASSIGNMENT($access) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $group : public span
{
	only(expr) expr;

	$group() = default;

	COPY_CONSTRUCTOR($group) = delete;
	MOVE_CONSTRUCTOR($group) = default;

	COPY_ASSIGNMENT($group) = delete;
	MOVE_ASSIGNMENT($group) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

struct $call : public span
{
	only(expr) call;
	many(expr) args;

	$call() = default;

	COPY_CONSTRUCTOR($call) = delete;
	MOVE_CONSTRUCTOR($call) = default;

	COPY_ASSIGNMENT($call) = delete;
	MOVE_ASSIGNMENT($call) = default;

	auto accept(const lang::visitor& impl) const { impl.visit(*this); }
};

namespace lang
{
	struct reflect : public lang::visitor
	{
		void visit(const only(decl)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(decl)& ast) const override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only(stmt)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(stmt)& ast) const override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only(expr)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(expr)& ast) const override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const only(node)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				this->visit(arg);
			},
			ast);
		}

		void visit(const many(node)& ast) const override
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		void visit(const many(body)& ast) const override
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
		void visit(const T& _) const
		{
			for (auto&& node : _)
			{ 
				this->visit(node);
			}
		}

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit(const $var& ast) const override
		{
			this->visit(ast.init);
		}

		void visit(const $fun& ast) const override
		{
			this->visit(ast.args);
			this->visit(ast.body);
		}

		void visit(const $trait& ast) const override
		{
			this->visit(ast.body);
		}

		void visit(const $class& ast) const override
		{
			this->visit(ast.body);
		}

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit(const $if& ast) const override
		{
			this->visit(ast.cases);
			this->visit(ast.block);
		}

		void visit(const $for& ast) const override
		{
			this->visit(ast.setup);
			this->visit(ast.input);
			this->visit(ast.after);
			this->visit(ast.block);
		}

		void visit(const $match& ast) const override
		{
			this->visit(ast.input);
			this->visit(ast.cases);
			this->visit(ast.block);
		}

		void visit(const $while& ast) const override
		{
			this->visit(ast.input);
			this->visit(ast.block);
		}

		void visit(const $break& ast) const override
		{
			// nothing to do here
		}

		void visit(const $return& ast) const override
		{
			this->visit(ast.value);
		}

		void visit(const $continue& ast) const override
		{
			// nothing to do here
		}

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit(const $unary& ast) const override
		{
			this->visit(ast.rhs);
		}

		void visit(const $binary& ast) const override 
		{
			this->visit(ast.lhs);
			this->visit(ast.rhs);
		}

		void visit(const $literal& ast) const override
		{
			// nothing to do here
		}

		void visit(const $symbol& ast) const override
		{
			// nothing to do here
		}

		void visit(const $access& ast) const override
		{
			// nothing to do here
		}

		void visit(const $group& ast) const override
		{
			this->visit(ast.expr);
		}

		void visit(const $call& ast) const override
		{
			this->visit(ast.args);
			this->visit(ast.call);
		}
	};

	struct printer : public lang::visitor
	{
		std::ostream& out;
		mutable size_t tab {0};

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

		auto gap() const
		{
			for (size_t i {0}; i < this->tab; ++i)
			{ /*-!-*/ this->out << "\t"; /*-!-*/ }
		}

	public:

		printer(decltype(out) out) : out {out} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		template
		<
			traits::printable T
		>
		void visit(const T& ast) const
		{
			this->out << ast << "\n";
		}

		void visit(const only(decl)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(decl)& ast) const override
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

		void visit(const only(stmt)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(stmt)& ast) const override
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

		void visit(const only(expr)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		void visit(const many(expr)& ast) const override
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

		void visit(const only(node)& ast) const override
		{
			std::visit([&](auto&& arg)
			{
				this->visit(arg);
			},
			ast);
		}

		void visit(const many(node)& ast) const override
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

		void visit(const many(body)& ast) const override
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
		void visit(const T& _) const
		{
			if (!_.empty())
			{
				size_t count {0};

				for (auto&& node : _)
				{
					this->visit(node);

					if (++count < _.size())
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

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit(const $var& ast) const override
		{
			START
			this->gap(); this->out << "[var]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "type: "; this->visit(ast.type);
			this->gap(); this->out << "init: "; this->visit(ast.init);
			CLOSE
		}

		void visit(const $fun& ast) const override
		{

			START
			this->gap(); this->out << "[fun]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "type: "; this->visit(ast.type);
			this->gap(); this->out << "body: "; this->visit(ast.body);
			CLOSE
		}

		void visit(const $trait& ast) const override
		{
			START
			this->gap(); this->out << "[trait]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "body: "; this->visit(ast.body);
			CLOSE
		}

		void visit(const $class& ast) const override
		{
			START
			this->gap(); this->out << "[class]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "body: "; this->visit(ast.body);
			CLOSE
		}

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit(const $if& ast) const override
		{
			START
			this->gap(); this->out << "[if]\n";
			this->gap(); this->out << "block: "; this->visit(ast.block);
			this->gap(); this->out << "cases: "; this->visit(ast.cases);
			CLOSE
		}

		void visit(const $for& ast) const override
		{
			START
			this->gap(); this->out << "[for]\n";
			this->gap(); this->out << "setup: "; this->visit(ast.setup);
			this->gap(); this->out << "input: "; this->visit(ast.input);
			this->gap(); this->out << "after: "; this->visit(ast.after);
			this->gap(); this->out << "block: "; this->visit(ast.block);
			CLOSE
		}

		void visit(const $match& ast) const override
		{
			START
			this->gap(); this->out << "[match]\n";
			this->gap(); this->out << "input: "; this->visit(ast.input);
			this->gap(); this->out << "block: "; this->visit(ast.block);
			this->gap(); this->out << "cases: "; this->visit(ast.cases);
			CLOSE
		}

		void visit(const $while& ast) const override
		{
			START
			this->gap(); this->out << "[while]\n";
			this->gap(); this->out << "input: "; this->visit(ast.input);
			this->gap(); this->out << "block: "; this->visit(ast.block);
			CLOSE
		}

		void visit(const $break& ast) const override
		{
			START
			this->gap(); this->out << "[break]\n";
			this->gap(); this->out << "label: "; this->visit(ast.label);
			CLOSE
		}

		void visit(const $return& ast) const override
		{
			START
			this->gap(); this->out << "[return]\n";
			this->gap(); this->out << "value: "; this->visit(ast.value);
			CLOSE
		}

		void visit(const $continue& ast) const override
		{
			START
			this->gap(); this->out << "[continue]\n";
			this->gap(); this->out << "label: "; this->visit(ast.label);
			CLOSE
		}

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit(const $unary& ast) const override
		{
			START
			this->gap(); this->out << "[unary]\n";
			this->gap(); this->out << "op : "; this->visit(ast.op);
			this->gap(); this->out << "rhs: "; this->visit(ast.rhs);
			CLOSE
		}

		void visit(const $binary& ast) const override
		{
			START
			this->gap(); this->out << "[binary]\n";
			this->gap(); this->out << "op : "; this->visit(ast.op);
			this->gap(); this->out << "lhs: "; this->visit(ast.lhs);
			this->gap(); this->out << "rhs: "; this->visit(ast.rhs);
			CLOSE
		}

		void visit(const $literal& ast) const override
		{
			START
			this->gap(); this->out << "[literal]\n";
			this->gap(); this->out << "type: "; this->visit(ast.type);
			this->gap(); this->out << "data: "; this->visit(ast.data);
			CLOSE
		}

		void visit(const $symbol& ast) const override
		{

			START
			this->gap(); this->out << "[symbol]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			CLOSE
		}

		void visit(const $access& data) const override
		{
			START
			this->gap(); this->out << "[access]\n";
			this->gap(); this->out << "expr: "; this->visit(data.expr);
			this->gap(); this->out << "type: "; this->visit(data.type);
			this->gap(); this->out << "name: "; this->visit(data.name);
			CLOSE
		}

		void visit(const $group& data) const override
		{
			START
			this->gap(); this->out << "[group]\n";
			this->gap(); this->out << "expr: "; this->visit(data.expr);
			CLOSE
		}

		void visit(const $call& data) const override
		{
			START
			this->gap(); this->out << "[call]\n";
			this->gap(); this->out << "call: "; this->visit(data.call);
			this->gap(); this->out << "args: "; this->visit(data.args);
			CLOSE
		}

		#undef START
		#undef CLOSE
	};
}

#undef only
#undef many

template
<
	type::string A,
	type::string B
>
struct program
{
	//|-------<chore>------|
	typedef error<A, B> fail;
	//|--------------------|

	std::vector<node> body;
	std::vector<fail> fault;

	program() = default;

	COPY_CONSTRUCTOR(program) = delete;
	MOVE_CONSTRUCTOR(program) = default;

	COPY_ASSIGNMENT(program) = delete;
	MOVE_ASSIGNMENT(program) = default;

	//|-----------------|
	//| member function |
	//|-----------------|

	auto run(const lang::visitor& impl) const
	{
		for (auto&& node : this->body)
		{
			impl.visit(node);
		}
	}
};
