#pragma once

#include <cassert>
#include <cstdint>
#include <utility>
#include <iomanip>
#include <iostream>

#include "models/str.hpp"

namespace lang
{
	#define fundamentals(macro) \
	/*|----------------|*/      \
	/*| signed integer |*/      \
	/*|----------------|*/      \
	macro(I8, u8"i8")           \
	macro(I16, u8"i16")         \
	macro(I32, u8"i32")         \
	macro(I64, u8"i64")         \
	/*|------------------|*/    \
	/*| unsigned integer |*/    \
	/*|------------------|*/    \
	macro(U8, u8"u8")           \
	macro(U16, u8"u16")         \
	macro(U32, u8"u32")         \
	macro(U64, u8"u64")         \
	/*|-----------------|*/     \
	/*| floating points |*/     \
	/*|-----------------|*/     \
	macro(F32, u8"f32")         \
	macro(F64, u8"f64")         \
	/*|------------------|*/    \
	/*| other data types |*/    \
	/*|------------------|*/    \
	macro(CODE, u8"code")       \
	macro(BOOL, u8"bool")       \
	macro(WORD, u8"word")       \
	/*|-----------------|*/     \
	/*| string storages |*/     \
	/*|-----------------|*/     \
	macro(UTF8, u8"utf8")       \
	macro(UTF16, u8"utf16")     \
	macro(UTF32, u8"utf32")     \

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
	macro(PTR_AT, u8"@")      \
	macro(PTR_OF, u8"&")      \
	/*|---------|*/           \
	/*| logical |*/           \
	/*|---------|*/           \
	macro(L_NOT, u8"!")       \
	/*|---------|*/           \
	/*| bitwise |*/           \
	/*|---------|*/           \
	macro(B_NOT, u8"not")     \

	#define operator_r(macro) \
	/*|--------|*/            \
	/*| callee |*/            \
	/*|--------|*/            \
	macro(CALL_M, u8".")      \
	macro(CALL_S, u8"?.")     \
	macro(CALL_U, u8"!.")     \
	macro(CALL_F, u8"::")     \

	#define operator_b(macro) \
	/*|------------|*/        \
	/*| assignment |*/        \
	/*|------------|*/        \
	macro(ASSIGN, u8"=")      \
	macro(ASSIGN_ADD, u8"+=") \
	macro(ASSIGN_SUB, u8"-=") \
	macro(ASSIGN_MUL, u8"*=") \
	macro(ASSIGN_DIV, u8"/=") \
	macro(ASSIGN_MOD, u8"%=") \
	macro(ASSIGN_POW, u8"^=") \
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
	macro(L_XOR, u8"~~")      \
	macro(L_AND, u8"&&")      \
	/*|---------|*/           \
	/*| bitwise |*/           \
	/*|---------|*/           \
	macro(B_OR, u8"or")       \
	macro(B_XOR, u8"xor")     \
	macro(B_AND, u8"and")     \
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
	macro(COALESCE, u8"??")   \

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
	macro(DEC, nullptr)     \
	macro(BIN, nullptr)     \
	macro(OCT, nullptr)     \
	macro(HEX, nullptr)     \
	/*|--------|*/          \
	/*| string |*/          \
	/*|--------|*/          \
	macro(CHAR, nullptr)    \
	macro(TEXT, nullptr)    \

}

enum class lexeme : uint8_t
#define macro(K, V) K,
{
	fundamentals(macro)
	delimeters(macro)
	operator_l(macro)
	operator_r(macro)
	operator_b(macro)
	keywords(macro)
	special(macro)
};
#undef macro

auto operator<<(std::ostream& os, const lexeme data) -> std::ostream&
{
	switch (data)
	{
		#define macro(K, V)  \
		/*|---------------|*/\
		case lexeme::K:      \
		{                    \
			return os << #K; \
		}                    \
		/*|---------------|*/\
	
		fundamentals(macro)
		delimeters(macro)
		operator_l(macro)
		operator_r(macro)
		operator_b(macro)
		keywords(macro)
		special(macro)
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
	type::string T
>
struct token
{
	//|---------------|
	//| the rule of 0 |
	//|---------------|

	lexeme type;
	//--[data]--//
	uint16_t x; //
	uint16_t y; //
	//----------//
	T::slice data;

	//|----------------------|
	//| traits::printable<T> |
	//|----------------------|

	friend auto operator<<(std::ostream& os, const token& token) -> std::ostream&
	{
		return
		(
			os
			<<
			"\033[30m" // set color
			<<
			"L"
			<<
			std::setfill('0') << std::setw(2) << token.y
			<<
			":"
			<<
			std::setfill('0') << std::setw(2) << token.x
			<<
			" "
			<<
			token.data
			<<
			"\033[0m" // reset color
		);
	}
};
