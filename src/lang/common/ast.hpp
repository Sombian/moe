#pragma once

#include <vector>
#include <memory>
#include <variant>
#include <cassert>
#include <cstdint>
#include <optional>
#include <iostream>

#include "./span.hpp"
#include "./token.hpp"
#include "./error.hpp"

#include "models/str.hpp"

#include "utils/convert.hpp"

#include "traits/rule_of_5.hpp"
#include "traits/visitable.hpp"

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

template
<
	typename A,
	typename B
>
inline constexpr auto is_l(const token<A, B>& tkn) -> bool
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
inline constexpr auto to_l(const token<A, B>& tkn) -> op_l
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
inline constexpr auto is_i(const token<A, B>& tkn) -> bool
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
inline constexpr auto to_i(const token<A, B>& tkn) -> op_i
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
inline constexpr auto is_r(const token<A, B>& tkn) -> bool
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
inline constexpr auto to_r(const token<A, B>& tkn) -> op_r
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

inline constexpr auto operator<<(std::ostream& os, const data value) -> std::ostream&
{
	switch (value)
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

inline constexpr auto operator<<(std::ostream& os, const op_l value) -> std::ostream&
{
	switch (value)
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

inline constexpr auto operator<<(std::ostream& os, const op_i value) -> std::ostream&
{
	switch (value)
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

inline constexpr auto operator<<(std::ostream& os, const op_r value) -> std::ostream&
{
	switch (value)
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

typedef std::variant
<
	std::unique_ptr<struct $fun>,
	std::unique_ptr<struct $var>,
	std::unique_ptr<struct $model>,
	std::unique_ptr<struct $trait>
>
decl;

typedef std::unique_ptr<$fun> fun_t;
typedef std::unique_ptr<$var> var_t;
typedef std::unique_ptr<$model> model_t;
typedef std::unique_ptr<$trait> trait_t;

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
	// decl
	std::unique_ptr<struct $fun>,
	std::unique_ptr<struct $var>,
	std::unique_ptr<struct $model>,
	std::unique_ptr<struct $trait>,
	// stmt
	std::unique_ptr<struct $if>,
	std::unique_ptr<struct $for>,
	std::unique_ptr<struct $match>,
	std::unique_ptr<struct $while>,
	std::unique_ptr<struct $break>,
	std::unique_ptr<struct $return>,
	std::unique_ptr<struct $continue>,
	// expr
	std::unique_ptr<struct $unary>,
	std::unique_ptr<struct $binary>,
	std::unique_ptr<struct $literal>,
	std::unique_ptr<struct $symbol>,
	std::unique_ptr<struct $access>,
	std::unique_ptr<struct $group>,
	std::unique_ptr<struct $call>
>
node;


#define only(T) /**/         T        /**/
#define many(T) /**/  std::vector<T>  /**/
#define some(T) /**/ std::optional<T> /**/

typedef std::vector<node> body;

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

static inline std::unique_ptr<llvm::LLVMContext> CONTEXT {std::make_unique<llvm::LLVMContext>(/*NONE*/)};
static inline std::unique_ptr<llvm::Module> MODULE {std::make_unique<llvm::Module>("moe.exe", *CONTEXT)};
static inline std::unique_ptr<llvm::IRBuilder<>> BUILDER {std::make_unique<llvm::IRBuilder<>>(*CONTEXT)};

//|---------------|
//| variant::decl |
//|---------------|

struct $fun : public span, public traits::visitable<$fun>
{
	//|---------|
	bool is_pure;
	//|---------|
	only(utf8) name;
	many($var) args;
	only(utf8) type;
	only(body) body;

	COPY_CONSTRUCTOR($fun) = delete;
	MOVE_CONSTRUCTOR($fun) = default;

	$fun() = default;

	COPY_ASSIGNMENT($fun) = delete;
	MOVE_ASSIGNMENT($fun) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $var : public span, public traits::visitable<$var>
{
	//|----------|
	bool is_const;
	//|----------|
	only(utf8) name;
	only(utf8) type;
	some(expr) init;

	COPY_CONSTRUCTOR($var) = delete;
	MOVE_CONSTRUCTOR($var) = default;

	$var() = default;

	COPY_ASSIGNMENT($var) = delete;
	MOVE_ASSIGNMENT($var) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $model : public span, public traits::visitable<$model>
{
	only(utf8) name;
	many($var) body;

	COPY_CONSTRUCTOR($model) = delete;
	MOVE_CONSTRUCTOR($model) = default;

	$model() = default;

	COPY_ASSIGNMENT($model) = delete;
	MOVE_ASSIGNMENT($model) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $trait : public span, public traits::visitable<$trait>
{
	only(utf8) name;
	many($fun) body;

	COPY_CONSTRUCTOR($trait) = delete;
	MOVE_CONSTRUCTOR($trait) = default;

	$trait() = default;

	COPY_ASSIGNMENT($trait) = delete;
	MOVE_ASSIGNMENT($trait) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

//|---------------|
//| variant::stmt |
//|---------------|

struct $if : public span, public traits::visitable<$if>
{
	many(expr) cases;
	many(body) block;

	COPY_CONSTRUCTOR($if) = delete;
	MOVE_CONSTRUCTOR($if) = default;

	$if() = default;

	COPY_ASSIGNMENT($if) = delete;
	MOVE_ASSIGNMENT($if) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $for : public span, public traits::visitable<$for>
{
	only(expr) setup;
	only(expr) input;
	only(expr) after;
	only(body) block;

	COPY_CONSTRUCTOR($for) = delete;
	MOVE_CONSTRUCTOR($for) = default;

	$for() = default;

	COPY_ASSIGNMENT($for) = delete;
	MOVE_ASSIGNMENT($for) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $match : public span, public traits::visitable<$match>
{
	only(expr) input;
	many(expr) cases;
	many(body) block;

	COPY_CONSTRUCTOR($match) = delete;
	MOVE_CONSTRUCTOR($match) = default;

	$match() = default;

	COPY_ASSIGNMENT($match) = delete;
	MOVE_ASSIGNMENT($match) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $while : public span, public traits::visitable<$while>
{
	only(expr) input;
	only(body) block;

	COPY_CONSTRUCTOR($while) = delete;
	MOVE_CONSTRUCTOR($while) = default;

	$while() = default;

	COPY_ASSIGNMENT($while) = delete;
	MOVE_ASSIGNMENT($while) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $break : public span, public traits::visitable<$break>
{
	some(utf8) label;

	COPY_CONSTRUCTOR($break) = delete;
	MOVE_CONSTRUCTOR($break) = default;

	$break() = default;

	COPY_ASSIGNMENT($break) = delete;
	MOVE_ASSIGNMENT($break) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $return : public span, public traits::visitable<$return>
{
	some(expr) value;

	COPY_CONSTRUCTOR($return) = delete;
	MOVE_CONSTRUCTOR($return) = default;

	$return() = default;

	COPY_ASSIGNMENT($return) = delete;
	MOVE_ASSIGNMENT($return) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $continue : public span, public traits::visitable<$continue>
{
	some(utf8) label;

	COPY_CONSTRUCTOR($continue) = delete;
	MOVE_CONSTRUCTOR($continue) = default;

	$continue() = default;

	COPY_ASSIGNMENT($continue) = delete;
	MOVE_ASSIGNMENT($continue) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

//|---------------|
//| variant::expr |
//|---------------|

struct $unary : public span, public traits::visitable<$unary>
{
	only(op_l) op;
	only(expr) rhs;

	COPY_CONSTRUCTOR($unary) = delete;
	MOVE_CONSTRUCTOR($unary) = default;

	$unary() = default;

	COPY_ASSIGNMENT($unary) = delete;
	MOVE_ASSIGNMENT($unary) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $binary : public span, public traits::visitable<$binary>
{
	only(op_i) op;
	only(expr) lhs;
	only(expr) rhs;

	COPY_CONSTRUCTOR($binary) = delete;
	MOVE_CONSTRUCTOR($binary) = default;

	$binary() = default;

	COPY_ASSIGNMENT($binary) = delete;
	MOVE_ASSIGNMENT($binary) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $literal : public span, public traits::visitable<$literal>
{
	only(data) type;
	only(utf8) data;

	COPY_CONSTRUCTOR($literal) = delete;
	MOVE_CONSTRUCTOR($literal) = default;

	$literal() = default;

	COPY_ASSIGNMENT($literal) = delete;
	MOVE_ASSIGNMENT($literal) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;

		switch (this->type)
		{
			//|----------------|
			//| signed integer |
			//|----------------|
			case data::I8:
			{
				return ConstantInt::get(Type::getInt8Ty(*CONTEXT), utils::stoi(this->data), true);
			}
			case data::I16:
			{
				return ConstantInt::get(Type::getInt16Ty(*CONTEXT), utils::stoi(this->data), true);
			}
			case data::I32:
			{
				return ConstantInt::get(Type::getInt32Ty(*CONTEXT), utils::stoi(this->data), true);
			}
			case data::I64:
			{
				return ConstantInt::get(Type::getInt64Ty(*CONTEXT), utils::stoi(this->data), true);
			}
			//|-----------------|
			//| floating points |
			//|-----------------|
			case data::F32:
			{
				return ConstantInt::get(Type::getFloatTy(*CONTEXT), utils::stof(this->data));
			}
			case data::F64:
			{
				return ConstantInt::get(Type::getDoubleTy(*CONTEXT), utils::stof(this->data));
			}
			//|------------------|
			//| unsigned integer |
			//|------------------|
			case data::U8:
			{
				return ConstantInt::get(Type::getInt8Ty(*CONTEXT), utils::stoi(this->data), false);
			}
			case data::U16:
			{
				return ConstantInt::get(Type::getInt16Ty(*CONTEXT), utils::stoi(this->data), false);
			}
			case data::U32:
			{
				return ConstantInt::get(Type::getInt32Ty(*CONTEXT), utils::stoi(this->data), false);
			}
			case data::U64:
			{
				return ConstantInt::get(Type::getInt64Ty(*CONTEXT), utils::stoi(this->data), false);
			}
		}
	}
};

struct $symbol : public span, public traits::visitable<$symbol>
{
	only(utf8) name;

	COPY_CONSTRUCTOR($symbol) = delete;
	MOVE_CONSTRUCTOR($symbol) = default;

	$symbol() = default;

	COPY_ASSIGNMENT($symbol) = delete;
	MOVE_ASSIGNMENT($symbol) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $access : public span, public traits::visitable<$access>
{
	only(op_r) type;
	only(expr) expr;
	only(utf8) name;

	COPY_CONSTRUCTOR($access) = delete;
	MOVE_CONSTRUCTOR($access) = default;

	$access() = default;

	COPY_ASSIGNMENT($access) = delete;
	MOVE_ASSIGNMENT($access) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $group : public span, public traits::visitable<$group>
{
	only(expr) expr;

	COPY_CONSTRUCTOR($group) = delete;
	MOVE_CONSTRUCTOR($group) = default;

	$group() = default;

	COPY_ASSIGNMENT($group) = delete;
	MOVE_ASSIGNMENT($group) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

struct $call : public span, public traits::visitable<$call>
{
	only(expr) call;
	many(expr) args;

	COPY_CONSTRUCTOR($call) = delete;
	MOVE_CONSTRUCTOR($call) = default;

	$call() = default;

	COPY_ASSIGNMENT($call) = delete;
	MOVE_ASSIGNMENT($call) = default;

	inline constexpr auto codegen() -> llvm::Value*
	{
		using namespace llvm;
	}
};

namespace lang
{
	struct reflect
	{
		virtual constexpr void visit(only(decl)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(decl)& ast)
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		virtual constexpr void visit(only(stmt)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(stmt)& ast)
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		virtual constexpr void visit(only(expr)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(expr)& ast)
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		virtual constexpr void visit(only(node)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(node)& ast)
		{
			for (auto&& node : ast)
			{
				this->visit(node);
			}
		}

		//|------------|
		//| edge cases |
		//|------------|

		template
		<
			typename T
		>
		inline constexpr void visit(many(T)& ast)
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
		inline constexpr void visit(some(T)& ast)
		{
			if (ast)
			{
				this->visit(ast.value());
			}
			else
			{
				// nothing to do here...
			}
		}

		//|---------------|
		//| variant::decl |
		//|---------------|

		virtual constexpr void visit($var& ast)
		{
			this->visit(ast.init);
		}

		virtual constexpr void visit($fun& ast)
		{
			this->visit(ast.args);
			this->visit(ast.body);
		}

		virtual constexpr void visit($trait& ast)
		{
			this->visit(ast.body);
		}

		virtual constexpr void visit($model& ast)
		{
			this->visit(ast.body);
		}

		//|---------------|
		//| variant::stmt |
		//|---------------|

		virtual constexpr void visit($if& ast)
		{
			this->visit(ast.cases);
			this->visit(ast.block);
		}

		virtual constexpr void visit($for& ast)
		{
			this->visit(ast.setup);
			this->visit(ast.input);
			this->visit(ast.after);
			this->visit(ast.block);
		}

		virtual constexpr void visit($match& ast)
		{
			this->visit(ast.input);
			this->visit(ast.cases);
			this->visit(ast.block);
		}

		virtual constexpr void visit($while& ast)
		{
			this->visit(ast.input);
			this->visit(ast.block);
		}

		virtual constexpr void visit($break& ast)
		{
			// nothing to do here
		}

		virtual constexpr void visit($return& ast)
		{
			this->visit(ast.value);
		}

		virtual constexpr void visit($continue& ast)
		{
			// nothing to do here
		}

		//|---------------|
		//| variant::expr |
		//|---------------|

		virtual constexpr void visit($unary& ast)
		{
			this->visit(ast.rhs);
		}

		virtual constexpr void visit($binary& ast) 
		{
			this->visit(ast.lhs);
			this->visit(ast.rhs);
		}

		virtual constexpr void visit($literal& ast)
		{
			// nothing to do here
		}

		virtual constexpr void visit($symbol& ast)
		{
			// nothing to do here
		}

		virtual constexpr void visit($access& ast)
		{
			// nothing to do here
		}

		virtual constexpr void visit($group& ast)
		{
			this->visit(ast.expr);
		}

		virtual constexpr void visit($call& ast)
		{
			this->visit(ast.args);
			this->visit(ast.call);
		}
	};

	struct printer
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

		virtual constexpr void visit(only(decl)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(decl)& ast)
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

		virtual constexpr void visit(only(stmt)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(stmt)& ast)
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

		virtual constexpr void visit(only(expr)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(expr)& ast)
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

		virtual constexpr void visit(only(node)& ast)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			ast);
		}

		virtual constexpr void visit(many(node)& ast)
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

		//|------------|
		//| edge cases |
		//|------------|

		template
		<
			typename T
		>
		inline constexpr void visit(many(T)& ast)
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
		inline constexpr void visit(some(T)& ast)
		{
			if (ast)
			{
				this->visit(ast.value());
			}
			else
			{
				this->out << "[empty]\n";
			}
		}


		inline constexpr void visit(auto& ast)
		{
			this->out << ast << "\n";
		}

		//|---------------|
		//| variant::decl |
		//|---------------|

		virtual constexpr void visit($fun& ast)
		{
			START
			this->gap(); this->out << "[fun]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "type: "; this->visit(ast.type);
			this->gap(); this->out << "body: "; this->visit(ast.body);
			CLOSE
		}

		virtual constexpr void visit($var& ast)
		{
			START
			this->gap(); this->out << "[var]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "type: "; this->visit(ast.type);
			this->gap(); this->out << "init: "; this->visit(ast.init);
			CLOSE
		}

		virtual constexpr void visit($model& ast)
		{
			START
			this->gap(); this->out << "[model]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "body: "; this->visit(ast.body);
			CLOSE
		}

		virtual constexpr void visit($trait& ast)
		{
			START
			this->gap(); this->out << "[trait]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			this->gap(); this->out << "body: "; this->visit(ast.body);
			CLOSE
		}

		//|---------------|
		//| variant::stmt |
		//|---------------|

		virtual constexpr void visit($if& ast)
		{
			START
			this->gap(); this->out << "[if]\n";
			this->gap(); this->out << "block: "; this->visit(ast.block);
			this->gap(); this->out << "cases: "; this->visit(ast.cases);
			CLOSE
		}

		virtual constexpr void visit($for& ast)
		{
			START
			this->gap(); this->out << "[for]\n";
			this->gap(); this->out << "setup: "; this->visit(ast.setup);
			this->gap(); this->out << "input: "; this->visit(ast.input);
			this->gap(); this->out << "after: "; this->visit(ast.after);
			this->gap(); this->out << "block: "; this->visit(ast.block);
			CLOSE
		}

		virtual constexpr void visit($match& ast)
		{
			START
			this->gap(); this->out << "[match]\n";
			this->gap(); this->out << "input: "; this->visit(ast.input);
			this->gap(); this->out << "block: "; this->visit(ast.block);
			this->gap(); this->out << "cases: "; this->visit(ast.cases);
			CLOSE
		}

		virtual constexpr void visit($while& ast)
		{
			START
			this->gap(); this->out << "[while]\n";
			this->gap(); this->out << "input: "; this->visit(ast.input);
			this->gap(); this->out << "block: "; this->visit(ast.block);
			CLOSE
		}

		virtual constexpr void visit($break& ast)
		{
			START
			this->gap(); this->out << "[break]\n";
			this->gap(); this->out << "label: "; this->visit(ast.label);
			CLOSE
		}

		virtual constexpr void visit($return& ast)
		{
			START
			this->gap(); this->out << "[return]\n";
			this->gap(); this->out << "value: "; this->visit(ast.value);
			CLOSE
		}

		virtual constexpr void visit($continue& ast)
		{
			START
			this->gap(); this->out << "[continue]\n";
			this->gap(); this->out << "label: "; this->visit(ast.label);
			CLOSE
		}

		//|---------------|
		//| variant::expr |
		//|---------------|

		virtual constexpr void visit($unary& ast)
		{
			START
			this->gap(); this->out << "[unary]\n";
			this->gap(); this->out << "op : "; this->visit(ast.op);
			this->gap(); this->out << "rhs: "; this->visit(ast.rhs);
			CLOSE
		}

		virtual constexpr void visit($binary& ast)
		{
			START
			this->gap(); this->out << "[binary]\n";
			this->gap(); this->out << "op : "; this->visit(ast.op);
			this->gap(); this->out << "lhs: "; this->visit(ast.lhs);
			this->gap(); this->out << "rhs: "; this->visit(ast.rhs);
			CLOSE
		}

		virtual constexpr void visit($literal& ast)
		{
			START
			this->gap(); this->out << "[literal]\n";
			this->gap(); this->out << "type: "; this->visit(ast.type);
			this->gap(); this->out << "data: "; this->visit(ast.data);
			CLOSE
		}

		virtual constexpr void visit($symbol& ast)
		{

			START
			this->gap(); this->out << "[symbol]\n";
			this->gap(); this->out << "name: "; this->visit(ast.name);
			CLOSE
		}

		virtual constexpr void visit($access& data)
		{
			START
			this->gap(); this->out << "[access]\n";
			this->gap(); this->out << "expr: "; this->visit(data.expr);
			this->gap(); this->out << "type: "; this->visit(data.type);
			this->gap(); this->out << "name: "; this->visit(data.name);
			CLOSE
		}

		virtual constexpr void visit($group& data)
		{
			START
			this->gap(); this->out << "[group]\n";
			this->gap(); this->out << "expr: "; this->visit(data.expr);
			CLOSE
		}

		virtual constexpr void visit($call& data)
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

template
<
	type::string A,
	type::string B
>
class program
{
	//|-------<chore>-------|
	typedef error<A, B> lint;
	//|---------------------|

public:

	only(body) body;
	many(lint) error;

	COPY_CONSTRUCTOR(program) = delete;
	MOVE_CONSTRUCTOR(program) = default;

	program() = default;

	COPY_ASSIGNMENT(program) = delete;
	MOVE_ASSIGNMENT(program) = default;

	template
	<
		typename V
	>
	inline constexpr auto visit(V& impl)
	{
		for (const auto& node : body)
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(impl);
			},
			node); // decl/stmt/expr
		}
	}

	inline constexpr auto compile(bool ir)
	{
		for (auto& _ : this->error)
		{
			std::cout << _ << '\n';
		}

		#ifndef NDEBUG //-------------------|
		MODULE->print(llvm::outs(), nullptr);
		#endif //---------------------------|
	}
};

#undef only
#undef many
#undef some
