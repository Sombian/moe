#pragma once

#include <variant>

#include "lang/lexer.hpp"

#include "./common/ast.hpp"
#include "./common/error.hpp"

template
<
	type::string A,
	type::string B
>
class parser
{
	lexer<A, B>& lexer;
	//---------[buffer]---------//
	decltype(lexer.pull()) out; //
	//--------------------------//

	#define E($value) error \
	{                       \
		.msg {$value},      \
		.x {this->out.x},   \
		.y {this->out.y},   \
	}                       \

public:

	//|---------------|
	//| the rule of 0 |
	//|---------------|

	parser(decltype(lexer) lexer) : lexer {lexer}, out {true} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	[[nodiscard]]
	auto pull() -> std::variant<program, bool, error>
	{
		program ast;
		return ast;
	}

	[[nodiscard]]
	auto print()
	{
		// TODO	
	}

private:

	//|------------|
	//| statements |
	//|------------|

	//|-------------|
	//| expressions |
	//|-------------|

	//|--------------|
	//| declarations |
	//|--------------|

	#undef E
};
