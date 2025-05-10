#pragma once

#include <memory>
#include <vector>
#include <cstdint>

#include "token.hpp"

#include "models/str.hpp"

namespace
{
	struct stmt_base { virtual ~stmt_base() = default; };
	struct expr_base { virtual ~expr_base() = default; };
	struct decl_base { virtual ~decl_base() = default; };

	//-------------------//|
	template<typename T> //|
	using single =       //|
	//----//-------------//|
	   T; //             //|
	//----//-------------//|
	template<typename T> //|
	using vector =       //|
	//-------------------//|
	std::vector<T>;      //|
	//-------------------//|
}

typedef std::unique_ptr<stmt_base> stmt;
typedef std::unique_ptr<expr_base> expr;
typedef std::unique_ptr<decl_base> decl;

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

enum class op_u : uint8_t
#define macro(K, V) K,
{
	operator_u(macro)
};
#undef macro

enum class op_b : uint8_t
#define macro(K, V) K,
{
	operator_b(macro)
};
#undef macro

enum class op_c : uint8_t
#define macro(K, V) K,
{
	operator_c(macro)
};
#undef macro

//|---------|
//| program |
//|---------|

struct program
{
	vector<decl> body;
};

//|------------|
//| statements |
//|------------|

namespace lang
{
	//|------|
	//| loop |
	//|------|

	struct _for final : public stmt_base
	{
		single<expr> a;
		single<expr> b;
		single<expr> c;
		single<stmt> block;
	};

	struct _while final : public stmt_base
	{
		single<expr> in;
		// single<expr> b;
		// single<expr> c;
		single<stmt> block;
	};

	//|--------|
	//| branch |
	//|--------|

	struct _if final : public stmt_base
	{
		single<expr> in;
		single<stmt> if_block;
		single<stmt> else_block;
		vector<stmt> else_if_block;
	};

	struct _match final : public stmt_base
	{
		single<expr> in;
		vector<expr> match_cases;
		vector<stmt> match_blocks;
	};

	//|---------|
	//| control |
	//|---------|

	struct _break final : public stmt_base
	{
		single<utf8> label;
	};

	struct _return final : public stmt_base
	{
		single<expr> value;
	};

	struct _continue final : public stmt_base
	{
		single<utf8> label;
	};
}

//|-------------|
//| expressions |
//|-------------|

namespace lang
{
	struct _unary final : public expr
	{
		single<op_u> op;
		single<expr> rhs;
	};

	struct _binary final : public expr
	{
		single<expr> lhs;
		single<op_b> op;
		single<expr> rhs;
	};

	struct _literal final : public expr
	{
		single<data> type;
		single<utf8> data;
	};

	struct _symbol final : public expr
	{
		single<utf8> name;
	};

	struct _group final : public expr
	{
		single<expr> body;
	};

	struct _call final : public expr
	{
		single<op_c> op;
		single<utf8> name;
		vector<expr> args;
	};
}

//|--------------|
//| declarations |
//|--------------|

namespace lang
{
	struct _var final : public decl_base
	{
		//-------------//
		bool is_const; //
		//-------------//
		single<utf8> name;
		single<data> type;
		single<expr> init;
	};
	struct _fun final : public decl_base
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
