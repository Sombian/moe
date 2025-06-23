#pragma once

#include <map>
#include <vector>
#include <memory>
#include <ranges>
#include <variant>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <optional>
#include <iostream>

#include "./span.hpp"
#include "./token.hpp"
#include "./error.hpp"

#include "models/str.hpp"

#include "utils/convert.hpp"

#include "traits/printable.hpp"
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
	std::unique_ptr<struct $fun>,
	std::unique_ptr<struct $var>,
	std::unique_ptr<struct $model>,
	std::unique_ptr<struct $trait>,
	std::unique_ptr<struct $if>,
	std::unique_ptr<struct $for>,
	std::unique_ptr<struct $match>,
	std::unique_ptr<struct $while>,
	std::unique_ptr<struct $break>,
	std::unique_ptr<struct $return>,
	std::unique_ptr<struct $continue>,
	std::unique_ptr<struct $unary>,
	std::unique_ptr<struct $binary>,
	std::unique_ptr<struct $literal>,
	std::unique_ptr<struct $symbol>,
	std::unique_ptr<struct $access>,
	std::unique_ptr<struct $group>,
	std::unique_ptr<struct $call>
>
node;

#define only(T)        T        
#define many(T) std::vector<T>  
#define some(T) std::optional<T>

typedef std::vector<node> body;

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"

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
};

struct $break : public span, public traits::visitable<$break>
{
	some(utf8) label;

	COPY_CONSTRUCTOR($break) = delete;
	MOVE_CONSTRUCTOR($break) = default;

	$break() = default;

	COPY_ASSIGNMENT($break) = delete;
	MOVE_ASSIGNMENT($break) = default;
};

struct $return : public span, public traits::visitable<$return>
{
	some(expr) value;

	COPY_CONSTRUCTOR($return) = delete;
	MOVE_CONSTRUCTOR($return) = default;

	$return() = default;

	COPY_ASSIGNMENT($return) = delete;
	MOVE_ASSIGNMENT($return) = default;
};

struct $continue : public span, public traits::visitable<$continue>
{
	some(utf8) label;

	COPY_CONSTRUCTOR($continue) = delete;
	MOVE_CONSTRUCTOR($continue) = default;

	$continue() = default;

	COPY_ASSIGNMENT($continue) = delete;
	MOVE_ASSIGNMENT($continue) = default;
};

//|---------------|
//| variant::expr |
//|---------------|

struct $unary : public span, public traits::visitable<$unary>
{
	only(op_l) opr;
	only(expr) rhs;

	COPY_CONSTRUCTOR($unary) = delete;
	MOVE_CONSTRUCTOR($unary) = default;

	$unary() = default;

	COPY_ASSIGNMENT($unary) = delete;
	MOVE_ASSIGNMENT($unary) = default;
};

struct $binary : public span, public traits::visitable<$binary>
{
	only(op_i) opr;
	only(expr) lhs;
	only(expr) rhs;

	COPY_CONSTRUCTOR($binary) = delete;
	MOVE_CONSTRUCTOR($binary) = default;

	$binary() = default;

	COPY_ASSIGNMENT($binary) = delete;
	MOVE_ASSIGNMENT($binary) = default;
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
};

struct $symbol : public span, public traits::visitable<$symbol>
{
	only(utf8) name;

	COPY_CONSTRUCTOR($symbol) = delete;
	MOVE_CONSTRUCTOR($symbol) = default;

	$symbol() = default;

	COPY_ASSIGNMENT($symbol) = delete;
	MOVE_ASSIGNMENT($symbol) = default;
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
};

struct $group : public span, public traits::visitable<$group>
{
	only(expr) expr;

	COPY_CONSTRUCTOR($group) = delete;
	MOVE_CONSTRUCTOR($group) = default;

	$group() = default;

	COPY_ASSIGNMENT($group) = delete;
	MOVE_ASSIGNMENT($group) = default;
};

struct $call : public span, public traits::visitable<$call>
{
	only(expr) self;
	many(expr) args;

	COPY_CONSTRUCTOR($call) = delete;
	MOVE_CONSTRUCTOR($call) = default;

	$call() = default;

	COPY_ASSIGNMENT($call) = delete;
	MOVE_ASSIGNMENT($call) = default;
};

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

template
<
	type::string A,
	type::string B
>
struct program
{
	//|-------<chore>-------|
	typedef error<A, B> segf;
	//|---------------------|

	only(body) entry;
	many(segf) issue;

	COPY_CONSTRUCTOR(program) = delete;
	MOVE_CONSTRUCTOR(program) = default;

	program() = default;

	COPY_ASSIGNMENT(program) = delete;
	MOVE_ASSIGNMENT(program) = default;

	inline constexpr auto compile()
	{
		#ifndef NDEBUG //--------------|
		program::printer()(this->entry);
		#endif //----------------------|

		for (auto&& error : this->issue)
		{
			std::cout << error << '\n';
		}

		llvm::LLVMContext context {       };
		llvm::IRBuilder<> builder {context};

		//|----------<lookup table>--------|
		std::map<utf8, llvm::Type*> registry;
		//|--------------------------------|

		llvm::Module module {"main", context};

		registry[u8"i8"] = llvm::Type::getInt8Ty(context);
		registry[u8"i16"] = llvm::Type::getInt16Ty(context);
		registry[u8"i32"] = llvm::Type::getInt32Ty(context);
		registry[u8"i64"] = llvm::Type::getInt64Ty(context);

		registry[u8"f32"] = llvm::Type::getFloatTy(context);
		registry[u8"f64"] = llvm::Type::getDoubleTy(context);

		registry[u8"u8"] = llvm::Type::getInt8Ty(context);
		registry[u8"u16"] = llvm::Type::getInt16Ty(context);
		registry[u8"u32"] = llvm::Type::getInt32Ty(context);
		registry[u8"u64"] = llvm::Type::getInt64Ty(context);

		registry[u8"none"] = llvm::Type::getVoidTy(context);

		program::bind(*this, context, builder, module, registry);
		program::lint(*this, context, builder, module, registry);
	}

private:

	static constexpr auto bind(program<A, B>& exe, llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<utf8, llvm::Type*>& registry)
	{
		//|--------------|
		//| step 1. decl |
		//|--------------|

		overrides
		(
			program::reflect(),

			[&](auto& self, const $model& ast) -> void
			{
				registry[ast.name] = llvm::StructType::create
				(
					//|-------------------------------------------|
					                    context,
					//|-------------------------------------------|
					reinterpret_cast<const char*>(ast.name.c_str())
				);
			}
		)
		(exe.entry);

		//|----------------|
		//| step 2. define |
		//|----------------|

		overrides
		(
			program::reflect(),

			[&](auto& self, const $model& ast) -> void
			{
				llvm::cast<llvm::StructType>(registry[ast.name])->setBody
				(
					(
						//|-----------------------------------------|
						                    ast.body
						//|-----------------------------------------|
						| std::views::transform([&](const $var& node)
						{
							return registry[node.type];
						})
						| std::ranges::to<std::vector<llvm::Type*>>()
					),
					false
				);
			}
		)
		(exe.entry);
	}

	static constexpr auto lint(program<A, B>& exe, llvm::LLVMContext& context, llvm::IRBuilder<>& builder, llvm::Module& module, std::map<utf8, llvm::Type*>& registry)
	{
		//|--------------|
		//| step 1. scan |
		//|--------------|

		// TODO

		//|----------------|
		//| step 2. report |
		//|----------------|

		// TODO
	}

	static constexpr auto reflect()
	{
		return anchor{visitor
		{
			//|-------------------------------------------<double dispatch>-------------------------------------------|
			[=](auto& self, const only(decl)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			[=](auto& self, const only(stmt)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			[=](auto& self, const only(expr)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			[=](auto& self, const only(node)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			//|-------------------------------------------------------------------------------------------------------|

			[=]<typename T>(auto& self, const many(T)& ast) -> void
			{
				for (auto& node : ast) { self(node); }
			},
			[=]<typename T>(auto& self, const some(T)& ast) -> void
			{
				if (/*⩊*/ ast /*⩊*/ ) { self(*ast); }
			},

			//|---------------|
			//| variant::decl |
			//|---------------|

			[=](auto& self, const $fun& ast) -> void
			{
				self(ast.args);
				self(ast.body);
			},
			[=](auto& self, const $var& ast) -> void
			{
				self(ast.init);
			},
			[=](auto& self, const $model& ast) -> void
			{
				self(ast.body);
			},
			[=](auto& self, const $trait& ast) -> void
			{
				self(ast.body);
			},

			//|---------------|
			//| variant::stmt |
			//|---------------|

			[=](auto& self, const $if& ast) -> void
			{
				self(ast.cases);
				self(ast.block);
			},
			[=](auto& self, const $for& ast) -> void
			{
				self(ast.setup);
				self(ast.input);
				self(ast.after);
				self(ast.block);
			},
			[=](auto& self, const $match& ast) -> void
			{
				self(ast.input);
				self(ast.cases);
				self(ast.block);
			},
			[=](auto& self, const $while& ast) -> void
			{
				self(ast.input);
				self(ast.block);
			},
			[=](auto& self, const $break& ast) -> void
			{
				// no inner ast
			},
			[=](auto& self, const $return& ast) -> void
			{
				self(ast.value);
			},
			[=](auto& self, const $continue& ast) -> void
			{
				// no inner ast
			},

			//|---------------|
			//| variant::expr |
			//|---------------|

			[=](auto& self, const $unary& ast) -> void
			{
				self(ast.rhs);
			},
			[=](auto& self, const $binary& ast) -> void
			{
				self(ast.lhs);
				self(ast.rhs);
			},
			[=](auto& self, const $literal& ast) -> void
			{
				// no inner ast
			},
			[=](auto& self, const $symbol& ast) -> void
			{
				// no inner ast
			},
			[=](auto& self, const $access& ast) -> void
			{
				self(ast.expr);
			},
			[=](auto& self, const $group& ast) -> void
			{
				self(ast.expr);
			},
			[=](auto& self, const $call& ast) -> void
			{
				self(ast.args);
				self(ast.self);
			}
		}};
	}

	static constexpr auto printer()
	{
		//|--------------<state>--------------|
		auto tab {std::make_shared<size_t>(0)};
		//|-----------------------------------|

		#define GAP                           \
		{                                     \
			for (size_t i {0}; i < *tab; ++i) \
			{/*⩊*/ std::cout << "\t"; /*⩊*/} \
		}                                     \

		#define START          /*|---------|*/\
		{                      /*| lorem.. |*/\
			std::cout << "\n"; /*| ..ipsum |*/\
			GAP; ++(*tab);     /*| lorem.. |*/\
			std::cout << "{{"; /*| ..ipsum |*/\
			std::cout << "\n"; /*| lorem.. |*/\
		}                      /*|---------|*/\

		#define CLOSE          /*|---------|*/\
		{                      /*| lorem.. |*/\
			--(*tab); GAP;     /*| ..ipsum |*/\
			std::cout << "}}"; /*| lorem.. |*/\
			std::cout << "\n"; /*| ..ipsum |*/\
		}                      /*|---------|*/\

		return anchor{visitor
		{
			//|-------------------------------------------<double dispatch>-------------------------------------------|
			[=](auto& self, const only(decl)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			[=](auto& self, const only(stmt)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			[=](auto& self, const only(expr)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			[=](auto& self, const only(node)& ast) -> void { std::visit([&](auto&& ptr) { ptr->accept(self); }, ast); },
			//|-------------------------------------------------------------------------------------------------------|

			[=](auto&, const traits::printable auto& ast) -> void
			{
				std::cout << "\033[33m" << ast << "\033[0m\n";
			},
			[=]<typename T>(auto& self, const many(T)& ast) -> void
			{
				if (!ast.empty())
				{
					size_t count {0};

					for (const auto& node : ast)
					{
						self(node); // <- recurse

						if (++count < ast.size())
						{
							GAP; std::cout << "&&";
						}
					}
				}
				else
				{
					std::cout << "\033[36m" << "none" << "\033[0m\n";
				}
			},
			[=]<typename T>(auto& self, const some(T)& ast) -> void
			{
				if (ast)
				{
					self(*ast); // <- unwrap std::optional & recurse
				}
				else
				{
					std::cout << "\033[36m" << "none" << "\033[0m\n";
				}
			},

			//|---------------|
			//| variant::decl |
			//|---------------|

			[=](auto& self, const $fun& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "fun" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "name: "; self(ast.name);
					GAP; std::cout << "args: "; self(ast.args);
					GAP; std::cout << "type: "; self(ast.type);
					GAP; std::cout << "body: "; self(ast.body);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $var& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "var" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "name: "; self(ast.name);
					GAP; std::cout << "type: "; self(ast.type);
					GAP; std::cout << "init: "; self(ast.init);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $model& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "model" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "name: "; self(ast.name);
					GAP; std::cout << "type: "; self(ast.body);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $trait& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "trait" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "name: "; self(ast.name);
					GAP; std::cout << "type: "; self(ast.body);
					//|---------------------------------------|
				}
				CLOSE;
			},

			//|---------------|
			//| variant::stmt |
			//|---------------|

			[=](auto& self, const $if& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "if" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "block: "; self(ast.block);
					GAP; std::cout << "cases: "; self(ast.cases);
					//|-----------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $for& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "for" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "setup: "; self(ast.setup);
					GAP; std::cout << "input: "; self(ast.input);
					GAP; std::cout << "after: "; self(ast.after);
					GAP; std::cout << "block: "; self(ast.block);
					//|-----------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $match& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "match" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "input: "; self(ast.input);
					GAP; std::cout << "block: "; self(ast.block);
					GAP; std::cout << "cases: "; self(ast.cases);
					//|-----------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $while& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "while" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "input: "; self(ast.input);
					GAP; std::cout << "block: "; self(ast.block);
					//|-----------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $break& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "break" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "label: "; self(ast.label);
					//|-----------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $return& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "return" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "value: "; self(ast.value);
					//|-----------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $continue& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "continue" << "\033[0m\n";
					//|-----------------------------------------|
					GAP; std::cout << "label: "; self(ast.label);
					//|-----------------------------------------|
				}
				CLOSE;
			},

			//|---------------|
			//| variant::expr |
			//|---------------|

			[=](auto& self, const $unary& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "unary" << "\033[0m\n";
					//|-------------------------------------|
					GAP; std::cout << "opr: "; self(ast.opr);
					GAP; std::cout << "rhs: "; self(ast.rhs);
					//|-------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $binary& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "binary" << "\033[0m\n";
					//|-------------------------------------|
					GAP; std::cout << "opr: "; self(ast.opr);
					GAP; std::cout << "lhs: "; self(ast.lhs);
					GAP; std::cout << "rhs: "; self(ast.rhs);
					//|-------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $literal& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "literal" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "type: "; self(ast.type);
					GAP; std::cout << "data: "; self(ast.data);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $symbol& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "symbol" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "name: "; self(ast.name);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $access& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "access" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "expr: "; self(ast.expr);
					GAP; std::cout << "type: "; self(ast.type);
					GAP; std::cout << "name: "; self(ast.name);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $group& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "group" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "expr: "; self(ast.expr);
					//|---------------------------------------|
				}
				CLOSE;
			},
			[=](auto& self, const $call& ast) -> void
			{
				START;
				{
					GAP; std::cout << "\033[36m" << "call" << "\033[0m\n";
					//|---------------------------------------|
					GAP; std::cout << "self: "; self(ast.self);
					GAP; std::cout << "args: "; self(ast.args);
					//|---------------------------------------|
				}
				CLOSE;
			}
		}};

		#undef START
		#undef CLOSE
	}
};

#undef only
#undef many
#undef some
