#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <cassert>
#include <cstdint>
#include <cstddef>

#include "token.hpp"

#include "models/str.hpp"
#include "traits/printable.hpp"
#include "traits/rule_of_5.hpp"

namespace lang
{
	struct $var;
	struct $fun;
	struct $trait;
	struct $class;
}

typedef std::variant
<
	std::unique_ptr<lang::$var>,
	std::unique_ptr<lang::$fun>,
	std::unique_ptr<lang::$trait>,
	std::unique_ptr<lang::$class>
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
	struct $unary;
	struct $binary;
	struct $literal;
	struct $symbol;
	struct $access;
	struct $group;
	struct $call;
}

typedef std::variant
<
	std::unique_ptr<lang::$unary>,
	std::unique_ptr<lang::$binary>,
	std::unique_ptr<lang::$literal>,
	std::unique_ptr<lang::$symbol>,
	std::unique_ptr<lang::$access>,
	std::unique_ptr<lang::$group>,
	std::unique_ptr<lang::$call>
>
expr;

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

namespace lang
{
	struct visitor
	{
		virtual void visit(const only<decl>&) = 0;
		virtual void visit(const many<decl>&) = 0;

		virtual void visit(const only<stmt>&) = 0;
		virtual void visit(const many<stmt>&) = 0;

		virtual void visit(const only<expr>&) = 0;
		virtual void visit(const many<expr>&) = 0;

		virtual void visit(const only<node>&) = 0;
		virtual void visit(const many<node>&) = 0;

		virtual void visit(const many<body>&) = 0;
		// decl
		virtual void visit(const $var&) = 0;
		virtual void visit(const $fun&) = 0;
		virtual void visit(const $trait&) = 0;
		virtual void visit(const $class&) = 0;
		// stmt
		virtual void visit(const $if&) = 0;
		virtual void visit(const $match&) = 0;
		virtual void visit(const $for&) = 0;
		virtual void visit(const $while&) = 0;
		virtual void visit(const $break&) = 0;
		virtual void visit(const $return&) = 0;
		virtual void visit(const $continue&) = 0;
		// expr
		virtual void visit(const $unary&) = 0;
		virtual void visit(const $binary&) = 0;
		virtual void visit(const $literal&) = 0;
		virtual void visit(const $symbol&) = 0;
		virtual void visit(const $access&) = 0;
		virtual void visit(const $group&) = 0;
		virtual void visit(const $call&) = 0;
	};
}

// decl
namespace lang
{
	struct $var
	{
		only<utf8> name;
		only<utf8> type;
		only<expr> init;
		//-------------//
		bool is_const; //
		//-------------//

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $fun
	{
		many<$var> args;
		only<utf8> name;
		only<utf8> type;
		only<body> body;
		//------------//
		bool is_pure; //
		//------------//

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $trait
	{
		only<utf8> name;
		many<$fun> body;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $class
	{
		only<utf8> name;
		many<$var> body;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};
}

// stmt
namespace lang
{
	struct $if
	{
		many<expr> cases;
		many<body> block;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $match
	{
		only<expr> input;
		many<expr> cases;
		many<body> block;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $for
	{
		only<expr> setup;
		only<expr> input;
		only<expr> after;
		only<body> block;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $while
	{
		only<expr> input;
		only<body> block;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};
	
	struct $break
	{
		only<utf8> label;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $return
	{
		only<expr> value;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $continue
	{
		only<utf8> label;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};
}

// expr
namespace lang
{
	struct $unary
	{
		only<op_l> lhs;
		only<expr> rhs;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $binary
	{
		only<expr> lhs;
		only<op_i> mhs;
		only<expr> rhs;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $literal
	{
		only<data> type;
		only<utf8> data;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $symbol
	{
		only<utf8> name;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $access
	{
		only<expr> expr;
		only<op_r> type;
		only<utf8> name;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $group
	{
		only<expr> expr;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $call
	{
		only<expr> call;
		many<expr> args;

		auto accept(visitor& impl) const { impl.visit(*this); }
	};
}

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
			assert(!!!"error");
			std::unreachable();
		}
	}
}

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
			assert(!!!"error");
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
			assert(!!!"error");
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
			assert(!!!"error");
			std::unreachable();
		}
	}
}

//|---------------|
//| "TIRO FINALE" |
//|---------------|

struct program
{
	class printer : lang::visitor
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

		auto gap()
		{
			for (size_t i {0}; i < this->tab; ++i)
			{ /*-!-*/ this->out << "\t"; /*-!-*/ }
		}

	public:

		printer(decltype(out) out) : out {out} {}

		//|-----------------|
		//| member function |
		//|-----------------|

		void visit(const traits::printable auto& data)
		{
			this->out << data << "\n";
		}

		void visit(const only<decl>& data) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			data);
		}

		void visit(const many<decl>& data) override
		{
			if (!data.empty())
			{
				size_t count {0};

				for (auto&& node : data)
				{
					this->visit(node);

					if (++count < data.size())
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

		void visit(const only<stmt>& data) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			data);
		}

		void visit(const many<stmt>& data) override
		{
			if (!data.empty())
			{
				size_t count {0};

				for (auto&& node : data)
				{
					this->visit(node);

					if (++count < data.size())
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

		void visit(const only<expr>& data) override
		{
			std::visit([&](auto&& arg)
			{
				arg->accept(*this);
			},
			data);
		}

		void visit(const many<expr>& data) override
		{
			if (!data.empty())
			{
				size_t count {0};

				for (auto&& node : data)
				{
					this->visit(node);

					if (++count < data.size())
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

		void visit(const only<node>& data) override
		{
			std::visit([&](auto&& arg)
			{
				this->visit(arg);
			},
			data);
		}

		void visit(const many<node>& data) override
		{
			if (!data.empty())
			{
				size_t count {0};

				for (auto&& node : data)
				{
					this->visit(node);

					if (++count < data.size())
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

		void visit(const many<body>& data) override
		{
			if (!data.empty())
			{
				size_t count {0};

				for (auto&& node : data)
				{
					this->visit(node);

					if (++count < data.size())
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
		
		#define VISIT($T)                      \
		                                       \
		void visit(const many<$T>& data)       \
		{                                      \
			if (!data.empty())                 \
			{                                  \
				size_t count {0};              \
				                               \
				for (auto&& node : data)       \
				{                              \
					this->visit(node);         \
					                           \
					if (++count < data.size()) \
					{                          \
						this->gap();           \
						/*-<seperator>-*/      \
						this->out << "&";      \
						/*-------------*/      \
					}                          \
				}                              \
			}                                  \
			else                               \
			{                                  \
				this->out << "[empty]\n";      \
			}                                  \
		}                                      \

		//|---------------|
		//| variant::decl |
		//|---------------|

		void visit(const lang::$var& data) override
		{
			START
			this->gap(); this->out << "[var]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			this->gap(); this->out << "type" << ": "; this->visit(data.type);
			this->gap(); this->out << "init" << ": "; this->visit(data.init);
			CLOSE
		}
		VISIT(lang::$var) // vector ver. codegen

		void visit(const lang::$fun& data) override
		{

			START
			this->gap(); this->out << "[fun]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			this->gap(); this->out << "type" << ": "; this->visit(data.type);
			this->gap(); this->out << "body" << ": "; this->visit(data.body);
			CLOSE
		}
		VISIT(lang::$fun) // vector ver. codegen

		void visit(const lang::$trait& data) override
		{
			START
			this->gap(); this->out << "[trait]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			this->gap(); this->out << "body" << ": "; this->visit(data.body);
			CLOSE
		}
		VISIT(lang::$trait) // vector ver. codegen

		void visit(const lang::$class& data) override
		{
			START
			this->gap(); this->out << "[class]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			this->gap(); this->out << "body" << ": "; this->visit(data.body);
			CLOSE
		}
		VISIT(lang::$class) // vector ver. codegen

		//|---------------|
		//| variant::stmt |
		//|---------------|

		void visit(const lang::$if& data) override
		{
			START
			this->gap(); this->out << "[if]" << "\n";
			this->gap(); this->out << "block" << ": "; this->visit(data.block);
			this->gap(); this->out << "cases" << ": "; this->visit(data.cases);
			CLOSE
		}
		VISIT(lang::$if) // vector ver. codegen

		void visit(const lang::$match& data) override
		{
			START
			this->gap(); this->out << "[match]" << "\n";
			this->gap(); this->out << "input" << ": "; this->visit(data.input);
			this->gap(); this->out << "block" << ": "; this->visit(data.block);
			this->gap(); this->out << "cases" << ": "; this->visit(data.cases);
			CLOSE
		}
		VISIT(lang::$match) // vector ver. codegen

		void visit(const lang::$for& data) override
		{
			START
			this->gap(); this->out << "[for]" << "\n";
			this->gap(); this->out << "setup" << ": "; this->visit(data.setup);
			this->gap(); this->out << "input" << ": "; this->visit(data.input);
			this->gap(); this->out << "after" << ": "; this->visit(data.after);
			this->gap(); this->out << "block" << ": "; this->visit(data.block);
			CLOSE
		}
		VISIT(lang::$for) // vector ver. codegen

		void visit(const lang::$while& data) override
		{
			START
			this->gap(); this->out << "[while]" << "\n";
			this->gap(); this->out << "input" << ": "; this->visit(data.input);
			this->gap(); this->out << "block" << ": "; this->visit(data.block);
			CLOSE
		}
		VISIT(lang::$while) // vector ver. codegen

		void visit(const lang::$break& data) override
		{
			START
			this->gap(); this->out << "[break]" << "\n";
			this->gap(); this->out << "label" << ": "; this->visit(data.label);
			CLOSE
		}
		VISIT(lang::$break) // vector ver. codegen

		void visit(const lang::$return& data) override
		{
			START
			this->gap(); this->out << "[return]" << "\n";
			this->gap(); this->out << "value" << ": "; this->visit(data.value);
			CLOSE
		}
		VISIT(lang::$return) // vector ver. codegen

		void visit(const lang::$continue& data) override
		{
			START
			this->gap(); this->out << "[continue]" << "\n";
			this->gap(); this->out << "label" << ": "; this->visit(data.label);
			CLOSE
		}
		VISIT(lang::$continue) // vector ver. codegen

		//|---------------|
		//| variant::expr |
		//|---------------|

		void visit(const lang::$unary& data) override
		{
			START
			this->gap(); this->out << "[unary]" << "\n";
			this->gap(); this->out << "lhs" << ": "; this->visit(data.lhs);
			this->gap(); this->out << "rhs" << ": "; this->visit(data.rhs);
			CLOSE
		}
		VISIT(lang::$unary) // vector ver. codegen

		void visit(const lang::$binary& data) override
		{
			START
			this->gap(); this->out << "[binary]" << "\n";
			this->gap(); this->out << "lhs" << ": "; this->visit(data.lhs);
			this->gap(); this->out << "mhs" << ": "; this->visit(data.mhs);
			this->gap(); this->out << "rhs" << ": "; this->visit(data.rhs);
			CLOSE
		}
		VISIT(lang::$binary) // vector ver. codegen

		void visit(const lang::$literal& data) override
		{
			START
			this->gap(); this->out << "[literal]" << "\n";
			this->gap(); this->out << "type" << ": "; this->visit(data.type);
			this->gap(); this->out << "data" << ": "; this->visit(data.data);
			CLOSE
		}
		VISIT(lang::$literal) // vector ver. codegen

		void visit(const lang::$symbol& data) override
		{

			START
			this->gap(); this->out << "[symbol]" << "\n";
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			CLOSE
		}
		VISIT(lang::$symbol) // vector ver. codegen

		void visit(const lang::$access& data) override
		{
			START
			this->gap(); this->out << "[access]" << "\n";
			this->gap(); this->out << "expr" << ": "; this->visit(data.expr);
			this->gap(); this->out << "type" << ": "; this->visit(data.type);
			this->gap(); this->out << "name" << ": "; this->visit(data.name);
			CLOSE
		}
		VISIT(lang::$access) // vector ver. codegen

		void visit(const lang::$group& data) override
		{
			START
			this->gap(); this->out << "[group]" << "\n";
			this->gap(); this->out << "expr" << ": "; this->visit(data.expr);
			CLOSE
		}
		VISIT(lang::$group) // vector ver. codegen

		void visit(const lang::$call& data) override
		{
			START
			this->gap(); this->out << "[call]" << "\n";
			this->gap(); this->out << "call" << ": "; this->visit(data.call);
			this->gap(); this->out << "args" << ": "; this->visit(data.args);
			CLOSE
		}
		VISIT(lang::$call) // vector ver. codegen

		#undef START
		#undef CLOSE
		#undef VISIT
	};

public:

	many<node> ast;

	program() = default;

	COPY_CONSTRUCTOR(program) = delete;
	MOVE_CONSTRUCTOR(program) = default;

	COPY_ASSIGNMENT(program) = delete;
	MOVE_ASSIGNMENT(program) = default;

	//|-----------------|
	//| member function |
	//|-----------------|

	auto print() const
	{
		printer impl {std::cout};

		for (auto&& node : this->ast)
		{
			std::visit([&](auto&& arg)
			{
				impl.visit(arg);
			},
			node);
		}
	}
};
