#pragma once

#include <vector>
#include <memory>
#include <cassert>
#include <cstdint>
#include <variant>
#include <optional>

#include "./span.hpp"
#include "./token.hpp"
#include "./error.hpp"

#include "models/str.hpp"

#include "traits/visitable.hpp"

enum class ty : uint8_t
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
	BOOL,
	WORD,
	CODE,
	TEXT,
	NONE,
};

#define macro($K, $V) $K,
enum class op : uint8_t
{
	operators(macro)
};
#undef macro

                  /**\----------------------------\**/
#define only(...) /**/        __VA_ARGS__         /**/
#define many(...) /**/  std::vector<__VA_ARGS__>  /**/
#define some(...) /**/ std::optional<__VA_ARGS__> /**/
                  /**\----------------------------\**/

using decl = std::variant
<
	std::unique_ptr<struct var_decl>,
	std::unique_ptr<struct fun_decl>,
	std::unique_ptr<struct model_decl>,
	std::unique_ptr<struct trait_decl>
>;

using stmt = std::variant
<
	std::unique_ptr<struct if_stmt>,
	std::unique_ptr<struct for_stmt>,
	std::unique_ptr<struct match_stmt>,
	std::unique_ptr<struct while_stmt>,
	std::unique_ptr<struct block_stmt>,
	std::unique_ptr<struct break_stmt>,
	std::unique_ptr<struct return_stmt>,
	std::unique_ptr<struct iterate_stmt>
>;

using expr = std::variant
<
	std::unique_ptr<struct prefix_expr>,
	std::unique_ptr<struct binary_expr>,
	std::unique_ptr<struct suffix_expr>,
	std::unique_ptr<struct access_expr>,
	std::unique_ptr<struct invoke_expr>,
	std::unique_ptr<struct literal_expr>,
	std::unique_ptr<struct symbol_expr>,
	std::unique_ptr<struct group_expr>
>;

using node = std::variant
<
	// decl
	std::unique_ptr<var_decl>,
	std::unique_ptr<fun_decl>,
	std::unique_ptr<model_decl>,
	std::unique_ptr<trait_decl>,
	// stmt
	std::unique_ptr<if_stmt>,
	std::unique_ptr<for_stmt>,
	std::unique_ptr<match_stmt>,
	std::unique_ptr<while_stmt>,
	std::unique_ptr<block_stmt>,
	std::unique_ptr<break_stmt>,
	std::unique_ptr<return_stmt>,
	std::unique_ptr<iterate_stmt>,
	// expr
	std::unique_ptr<prefix_expr>,
	std::unique_ptr<binary_expr>,
	std::unique_ptr<suffix_expr>,
	std::unique_ptr<access_expr>,
	std::unique_ptr<invoke_expr>,
	std::unique_ptr<literal_expr>,
	std::unique_ptr<symbol_expr>,
	std::unique_ptr<group_expr>
>;

template
<
	class A,
	class B
>
struct program
{
	typedef error<A, B> segf;

	many(node) body;
	many(segf) lint;
};

struct var_decl : public span,
public visitable<var_decl>
{
	only(bool) only;
	only(utf8) name;
	only(utf8) type;
	some(expr) init;
};

struct fun_decl : public span,
public visitable<fun_decl>
{
	struct data
	{
		only(utf8) name;
		only(utf8) type;
	};
	only(bool) pure;
	only(utf8) name;
	many(data) args;
	only(utf8) type;
	many(node) body;
};

struct model_decl : public span,
public visitable<model_decl>
{
	struct data
	{
		only(utf8) name;
		only(utf8) type;
	};
	only(utf8) name;
	many(data) body;
};

struct trait_decl : public span,
public visitable<trait_decl>
{
	struct data
	{
		only(utf8) name;
		many(utf8) args;
		only(utf8) type;
		many(node) body;
	};
	only(utf8) name;
	many(data) body;
};

struct if_stmt : public span,
public visitable<if_stmt>
{
	struct flow
	{
		only(expr) _if_;
		only(stmt) then;
	};
	many(flow) body;
};

struct for_stmt : public span,
public visitable<for_stmt>
{
	only(expr) init;
	only(expr) _if_;
	only(expr) task;
	only(stmt) body;
};

struct match_stmt : public span,
public visitable<match_stmt>
{
	struct flow
	{
		only(expr) _if_;
		only(stmt) then;
	};
	only(expr) data;
	many(flow) body;
};

struct while_stmt : public span,
public visitable<while_stmt>
{
	only(expr) _if_;
	only(stmt) body;
};

struct block_stmt : public span,
public visitable<block_stmt>
{
	many(node) body;
};

struct break_stmt : public span,
public visitable<break_stmt>
{
	only(utf8) label;
};

struct return_stmt : public span,
public visitable<return_stmt>
{
	only(expr) value;
};

struct iterate_stmt : public span,
public visitable<iterate_stmt>
{
	only(utf8) label;
};

struct prefix_expr : public span,
public visitable<prefix_expr>
{
	only(op) op;
	only(expr) rhs;
};

struct binary_expr : public span,
public visitable<binary_expr>
{
	only(expr) lhs;
	only(op) op;
	only(expr) rhs;
};

struct suffix_expr : public span,
public visitable<suffix_expr>
{
	only(expr) lhs;
	only(op) oper;
};

struct access_expr : public span,
public visitable<access_expr>
{
	only(expr) lhs;
	only(utf8) name;
};

struct invoke_expr : public span,
public visitable<invoke_expr>
{
	only(expr) lhs;
	many(expr) args;
};

struct literal_expr : public span,
public visitable<literal_expr>
{
	only(ty) type;
	only(utf8) self;
};

struct symbol_expr : public span,
public visitable<symbol_expr>
{
	only(utf8) self;
};

struct group_expr : public span,
public visitable<group_expr>
{
	only(expr) self;
};

#undef only
#undef some
#undef many
