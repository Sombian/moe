#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <iomanip>
#include <iostream>

#include "core/fs.hpp"

#include "./span.hpp"

#include "models/str.hpp"

namespace lang
{
	#define delimeters(macro) \
	/*|------------|*/        \
	/*| punctuator |*/        \
	/*|------------|*/        \
	macro(COMMA, u8",")       \
	macro(COLON, u8":")       \
	macro(S_COLON, u8";")     \
	macro(L_PAREN, u8"(")     \
	macro(R_PAREN, u8")")     \
	macro(L_BRACE, u8"{")     \
	macro(R_BRACE, u8"}")     \
	macro(L_BRACK, u8"[")     \
	macro(R_BRACK, u8"]")     \

	#define operator_l(macro) \
	/*|---------|*/           \
	/*| pointer |*/           \
	/*|---------|*/           \
	macro(AD_AT, u8"@")       \
	macro(AD_OF, u8"&")       \
	/*|---------|*/           \
	/*| logical |*/           \
	/*|---------|*/           \
	macro(L_NOT, u8"!")       \
	/*|---------|*/           \
	/*| bitwise |*/           \
	/*|---------|*/           \
	macro(B_NOT, u8"not")     \

	#define operator_i(macro) \
	/*|------------|*/        \
	/*| assignment |*/        \
	/*|------------|*/        \
	macro(ASSIGN, u8"=")      \
	macro(ADD_EQ, u8"+=")     \
	macro(SUB_EQ, u8"-=")     \
	macro(MUL_EQ, u8"*=")     \
	macro(DIV_EQ, u8"/=")     \
	macro(MOD_EQ, u8"%=")     \
	macro(POW_EQ, u8"^=")     \
	/*|------------|*/        \
	/*| arithmetic |*/        \
	/*|------------|*/        \
	macro(ADD, u8"+")         \
	macro(SUB, u8"-")         \
	macro(MUL, u8"*")         \
	macro(DIV, u8"/")         \
	macro(MOD, u8"%")         \
	macro(POW, u8"^")         \
	/*|---------|*/           \
	/*| logical |*/           \
	/*|---------|*/           \
	macro(L_OR, u8"||")       \
	macro(L_AND, u8"&&")      \
	/*|---------|*/           \
	/*| bitwise |*/           \
	/*|---------|*/           \
	macro(B_OR, u8"or")       \
	macro(B_AND, u8"and")     \
	macro(B_XOR, u8"xor")     \
	/*|-------|*/             \
	/*| shift |*/             \
	/*|-------|*/             \
	macro(SHL, u8"shl")       \
	macro(SHR, u8"shr")       \
	/*|----------|*/          \
	/*| equality |*/          \
	/*|----------|*/          \
	macro(EQ, u8"==")         \
	macro(NE, u8"!=")         \
	/*|------------|*/        \
	/*| relational |*/        \
	/*|------------|*/        \
	macro(LT, u8"<")          \
	macro(GT, u8">")          \
	macro(LTE, u8"<=")        \
	macro(GTE, u8">=")        \
	/*|------------|*/        \
	/*| ptr safety |*/        \
	/*|------------|*/        \
	macro(NIL, u8"??")        \

	#define operator_r(macro) \
	/*|--------|*/            \
	/*| access |*/            \
	/*|--------|*/            \
	macro(ACCESS_F, u8".")    \
	macro(ACCESS_S, u8"?.")   \
	macro(ACCESS_U, u8"!.")   \
	macro(ACCESS_N, u8"::")   \

	#define keywords(macro)       \
	/*|---------|*/               \
	/*| daclare |*/               \
	/*|---------|*/               \
	macro(STRUCT, u8"struct")     \
	macro(UNION, u8"union")       \
	macro(TRAIT, u8"trait")       \
	macro(ENUM, u8"enum")         \
	macro(IMPL, u8"impl")         \
	/*|--------|*/                \
	/*| define |*/                \
	/*|--------|*/                \
	macro(FUN, u8"fun")           \
	macro(FUN$, u8"fun!")         \
	macro(LET, u8"let")           \
	macro(LET$, u8"let!")         \
	/*|--------|*/                \
	/*| branch |*/                \
	/*|--------|*/                \
	macro(IF, u8"if")             \
	macro(ELSE, u8"else")         \
	macro(MATCH, u8"match")       \
	macro(CASE, u8"case")         \
	/*|------|*/                  \
	/*| loop |*/                  \
	/*|------|*/                  \
	macro(FOR, u8"for")           \
	macro(WHILE, u8"while")       \
	/*|-------|*/                 \
	/*| error |*/                 \
	/*|-------|*/                 \
	macro(TRY, u8"try")           \
	macro(CATCH, u8"catch")       \
	macro(FINALLY, u8"finally")   \
	/*|------|*/                  \
	/*| flow |*/                  \
	/*|------|*/                  \
	macro(BREAK, u8"break")       \
	macro(THROW, u8"throw")       \
	macro(RETURN, u8"return")     \
	macro(CONTINUE, u8"continue") \
	/*|--------|*/                \
	/*| module |*/                \
	/*|--------|*/                \
	macro(IMPORT, u8"import")     \
	macro(EXPORT, u8"export")     \
	/*|----------|*/              \
	/*| callable |*/              \
	/*|----------|*/              \
	macro(THIS, u8"this")         \
	macro(SUPER, u8"super")       \
	/*|----------|*/              \
	/*| compiler |*/              \
	/*|----------|*/              \
	macro(STATIC, u8"@static")    \
	macro(INLINE, u8"@inline")    \

	#define special(macro)  \
	/*|----|*/              \
	/*| id |*/              \
	/*|----|*/              \
	macro(SYMBOL, nullptr)  \
	/*|------|*/            \
	/*| null |*/            \
	/*|------|*/            \
	macro(NONE, u8"null")   \
	/*|------|*/            \
	/*| bool |*/            \
	/*|------|*/            \
	macro(TRUE, u8"true")   \
	macro(FALSE, u8"false") \
	/*|--------|*/          \
	/*| number |*/          \
	/*|--------|*/          \
	macro(INT, nullptr)     \
	macro(BIN, nullptr)     \
	macro(OCT, nullptr)     \
	macro(HEX, nullptr)     \
	macro(DEC, nullptr)     \
	/*|--------|*/          \
	/*| string |*/          \
	/*|--------|*/          \
	macro(CHAR, nullptr)    \
	macro(TEXT, nullptr)    \

}

enum class atom : uint8_t
#define macro(K, V) K,
{
	delimeters(macro)
	operator_l(macro)
	operator_i(macro)
	operator_r(macro)
	keywords(macro)
	special(macro)
};
#undef macro

auto operator<<(std::ostream& os, const atom data) -> std::ostream&
{
	switch (data)
	{
		#define macro(K, V)  \
		/*|---------------|*/\
		case atom::K:        \
		{                    \
			return os << #K; \
		}                    \
		/*|---------------|*/\

		delimeters(macro)
		operator_l(macro)
		operator_i(macro)
		operator_r(macro)
		keywords(macro)
		special(macro)
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
struct token : public span
{
	// SFINE: use T::slice if T is a string impl, otherwise use T directly
	typedef std::conditional_t<type::string_impl<B>, typename B::slice, B> view;

	//|-----<file>-----|
	fs::file<A, B>* src;
	//|----------------|
	atom type;
	view data;

public:

	token
	(
		decltype(x) x,
		decltype(y) y,
		decltype(src) src,
		decltype(type) type,
		decltype(data) data
	)
	:
	span {x, y}, src {src}, type {type}, data {data} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	friend auto operator==(const token& lhs, const atom rhs) -> bool
	{
		return lhs.type == rhs;
	}

	friend auto operator!=(const token& lhs, const atom rhs) -> bool
	{
		return lhs.type != rhs;
	}

	//|----------------------|
	//| traits::printable<T> |
	//|----------------------|

	friend auto operator<<(std::ostream& os, const token& token) -> std::ostream&
	{
		return
		(
			os
			<<
			"\033[35m" // set color
			<<
			token.src->path
			<<
			"("
			<<
			std::setfill('0') << std::setw(2) << token.y + 0
			<<
			","
			<<
			std::setfill('0') << std::setw(2) << token.x + 1
			<<
			")"
			<<
			" "
			<<
			token.data
			<<
			"\033[0m" // reset color
		);
	}
};
