#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <cassert>
#include <cstdint>

#include "token.hpp"

#include "models/str.hpp"

namespace lang
{
	struct $var;
	struct $fun;
}

typedef std::variant
<
	std::unique_ptr<lang::$var>,
	std::unique_ptr<lang::$fun>
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
	struct $unary_l;
	struct $binary;
	struct $unary_r;
	struct $literal;
	struct $symbol;
	struct $group;
	struct $call;
}

typedef std::variant
<
	std::unique_ptr<lang::$unary_l>,
	std::unique_ptr<lang::$binary>,
	std::unique_ptr<lang::$unary_r>,
	std::unique_ptr<lang::$literal>,
	std::unique_ptr<lang::$symbol>,
	std::unique_ptr<lang::$group>,
	std::unique_ptr<lang::$call>
>
expr;

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

//|-----------|
//| operators |
//|-----------|

namespace op
{
	template<typename B>
	auto is_l(const token<B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return true;     \
			}                    \
			/*|---------------|*/\
		
			case lexeme::ADD:
			{
				return true;
			}
			case lexeme::SUB:
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

	template<typename B>
	auto l(const token<B>& tkn) -> op_l
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
			{                    \
				return op_l::K;  \
			}                    \
			/*|---------------|*/\
		
			case lexeme::ADD:
			{
				return op_l::ADD;
			}
			case lexeme::SUB:
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

	template<typename B>
	auto is_i(const token<B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
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

	template<typename B>
	auto i(const token<B>& tkn) -> op_i
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
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

	template<typename B>
	auto is_r(const token<B>& tkn) -> bool
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
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

	template<typename B>
	auto r(const token<B>& tkn) -> op_r
	{
		switch (tkn.type)
		{
			#define macro(K, V)  \
			/*|---------------|*/\
			case lexeme::K:      \
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

//|--------------|
//| declarations |
//|--------------|

namespace lang
{
	struct $var
	{
		//-------------//
		bool is_const; //
		//-------------//
		only<utf8> name;
		only<utf8> type;
		only<expr> init;
	};
	struct $fun
	{
		//------------//
		bool is_pure; //
		//------------//
		many<$var> args;
		only<utf8> name;
		only<utf8> type;
		only<body> body;
	};
}

//|------------|
//| statements |
//|------------|

namespace lang
{
	struct $if
	{
		many<expr> cases;
		many<body> block;
	};

	struct $match
	{
		only<expr> input;
		many<expr> cases;
		many<body> block;
	};

	struct $for
	{
		only<expr> setup;
		only<expr> input;
		only<expr> after;
		only<body> block;
	};

	struct $while
	{
		only<expr> input;
		only<body> block;
	};
	
	struct $break
	{
		only<utf8> label;
	};

	struct $return
	{
		only<expr> value;
	};

	struct $continue
	{
		only<utf8> label;
	};
}

//|-------------|
//| expressions |
//|-------------|

namespace lang
{
	struct $unary_l
	{
		only<op_l> lhs;
		only<expr> rhs;
	};

	struct $binary
	{
		only<expr> lhs;
		only<op_i> mhs;
		only<expr> rhs;
	};

	struct $unary_r
	{
		only<expr> lhs;
		only<op_r> rhs;
	};

	struct $literal
	{
		only<data> type;
		only<utf8> data;
	};

	struct $symbol
	{
		only<utf8> name;
	};

	struct $group
	{
		only<expr> expr;
	};

	struct $call
	{
		only<op_r> type;
		only<utf8> name;
		many<expr> args;
	};
}

//|---------------|
//| "TIRO FINALE" |
//|---------------|

struct program
{
	many<node> ast;
};
