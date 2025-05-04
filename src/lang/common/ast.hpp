#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "token.hpp"

#include "models/str.hpp"

struct ast
{
	// codegen	
};
struct stmt : ast { virtual ~stmt() = default; };
struct expr : ast { virtual ~expr() = default; };
struct decl : ast { virtual ~decl() = default; };

namespace
{
	//-------------------------------------//
	template<typename T>                   //
	using single = std::unique_ptr<T>;     //
	//-------------------------------------//
	template<typename T>                   //
	using vector = std::vector<single<T>>; //
	//-------------------------------------//
}

#define macro(K, V) K,
enum class data : uint8_t
{
	fundamentals(macro)
};
#undef macro

enum class op_l : uint8_t
#define macro(K, V) K,
{
	operator_l(macro)
};
#undef macro

enum class op_r : uint8_t
#define macro(K, V) K,
{
	operator_r(macro)
};
#undef macro

enum class op_b : uint8_t
#define macro(K, V) K,
{
	operator_b(macro)
};
#undef macro

//|---------|
//| program |
//|---------|

struct program
{
	vector<stmt> body;
};

//|------------|
//| statements |
//|------------|

namespace lang
{
	//|------|
	//| loop |
	//|------|

	struct _for final : stmt
	{
		single<expr> a;
		single<expr> b;
		single<expr> c;
		single<stmt> body;
	};

	struct _while final : stmt
	{
		single<expr> a;
		// single<expr> b;
		// single<expr> c;
		single<stmt> body;
	};

	//|--------|
	//| branch |
	//|--------|

	struct _if final : stmt
	{
		single<expr> in;
		single<stmt> if_block;
		single<stmt> else_block;
		vector<stmt> else_if_block;
	};

	struct _match final : stmt
	{
		single<expr> in;
		vector<expr> match_cases;
		vector<stmt> match_blocks;
	};

	//|---------|
	//| control |
	//|---------|

	struct _break final : stmt
	{
		single<utf8> label;
	};

	struct _return final : stmt
	{
		single<expr> value;
	};

	struct _continue final : stmt
	{
		single<utf8> label;
	};
}

//|-------------|
//| expressions |
//|-------------|

namespace lang
{
	struct _unary_l final : expr
	{
		single<op_l> op;
		single<expr> rhs;
	};

	struct _unary_r final : expr
	{
		single<op_r> op;
		single<expr> lhs;
	};

	struct _binary final : expr
	{
		single<op_b> op;
		single<expr> lhs;
		single<expr> rhs;
	};

	struct _literal final : expr
	{
		single<data> type;
		single<utf8> data;
	};

	struct _symbol final : expr
	{
		single<utf8> name;
	};

	struct _group final : expr
	{
		single<expr> body;
	};

	struct _call final : expr
	{
		single<utf8> name;
		vector<expr> args;
	};
}

//|--------------|
//| declarations |
//|--------------|

namespace lang
{
	struct _var final : decl
	{
		//-------------//
		bool is_const; //
		//-------------//
		single<utf8> name;
		single<data> type;
		single<expr> init;
	};
	struct _fun final : decl
	{
		//------------//
		bool is_pure; //
		//------------//
		vector<_var> args;
		single<utf8> name;
		single<data> type;
		vector<stmt> body;
	};
}
