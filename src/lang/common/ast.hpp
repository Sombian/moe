#pragma once

#include <memory>
#include <vector>
#include <variant>
#include <cassert>
#include <cstdint>
#include <cstddef>

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
	struct $unary;
	struct $binary;
	struct $literal;
	struct $symbol;
	struct $group;
	struct $call;
}

typedef std::variant
<
	std::unique_ptr<lang::$unary>,
	std::unique_ptr<lang::$binary>,
	std::unique_ptr<lang::$literal>,
	std::unique_ptr<lang::$symbol>,
	std::unique_ptr<lang::$group>,
	std::unique_ptr<lang::$call>
>
expr;

namespace lang
{
	struct visitor
	{
		// decl
		virtual void visit(const $var&) = 0;
		virtual void visit(const $fun&) = 0;
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
		virtual void visit(const $group&) = 0;
		virtual void visit(const $call&) = 0;
	};
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

// lhs(prefix) operator
enum class op_l : uint8_t
#define macro(K, V) K,
{
	ADD, // prefix ver.
	SUB, // prefix ver.
	operator_l(macro)
};
#undef macro

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

// mhs(infix) operator
enum class op_i : uint8_t
#define macro(K, V) K,
{
	operator_i(macro)
};
#undef macro

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

// rhs(postfix) operator
enum class op_r : uint8_t
#define macro(K, V) K,
{
	operator_r(macro)
};
#undef macro

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

// decl
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

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
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

		// visitor pattern
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

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $match
	{
		only<expr> input;
		many<expr> cases;
		many<body> block;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $for
	{
		only<expr> setup;
		only<expr> input;
		only<expr> after;
		only<body> block;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $while
	{
		only<expr> input;
		only<body> block;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};
	
	struct $break
	{
		only<utf8> label;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $return
	{
		only<expr> value;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $continue
	{
		only<utf8> label;

		// visitor pattern
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

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $binary
	{
		only<expr> lhs;
		only<op_i> mhs;
		only<expr> rhs;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $literal
	{
		only<data> type;
		only<utf8> data;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $symbol
	{
		only<utf8> name;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $group
	{
		only<expr> expr;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};

	struct $call
	{
		only<utf8> name;
		only<op_r> type;
		many<expr> args;

		// visitor pattern
		auto accept(visitor& impl) const { impl.visit(*this); }
	};
}

//|---------------|
//| "TIRO FINALE" |
//|---------------|

struct program
{
	many<node> ast;

public:

	auto print() const
	{
		// symbolic expression printer
		class printer final : lang::visitor
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
				{ /*------*/ this->out << "\t"; /*------*/ }
			}

		public:

			printer(decltype(out) out) : out {out} {}

			//|---------|
			//| helpers |
			//|---------|

			auto visit(const only<decl>& data)
			{
				std::visit([&](auto&& arg)
				{
					arg->accept(*this);
				},
				data);
			}

			auto visit(const only<stmt>& data)
			{
				std::visit([&](auto&& arg)
				{
					arg->accept(*this);
				},
				data);
			}

			auto visit(const only<expr>& data)
			{
				std::visit([&](auto&& arg)
				{
					arg->accept(*this);
				},
				data);
			}

			auto visit(const only<node>& data)
			{
				std::visit([&](auto&& arg)
				{
					this->visit(arg);
				},
				data);
			}

			auto visit(const many<decl>& data)
			{
				for (const auto& node : data)
				{
					this->visit(node);
				}
			}

			auto visit(const many<stmt>& data)
			{
				for (const auto& node : data)
				{
					this->visit(node);
				}
			}

			auto visit(const many<expr>& data)
			{
				for (const auto& node : data)
				{
					this->visit(node);
				}
			}

			auto visit(const many<node>& data)
			{
				for (const auto& node : data)
				{
					this->visit(node);
				}
			}

			auto visit(const many<body>& data)
			{
				for (const auto& node : data)
				{
					this->visit(node);
				}
			}

			//|------|
			//| decl |
			//|------|

			void visit(const lang::$var& data) override
			{
				START
				this->gap(); this->out << "[var]" << "\n";
				this->gap(); this->out << "name" << ": " << data.name << "\n";
				this->gap(); this->out << "type" << ": " << data.type << "\n";
				this->gap(); this->out << "init" << ": "; this->visit(data.init);
				CLOSE
			}

			void visit(const lang::$fun& data) override
			{
	
				START
				this->gap(); this->out << "[fun]" << "\n";
				this->gap(); this->out << "name" << ": " << data.name << "\n";
				this->gap(); this->out << "type" << ": " << data.type << "\n";
				this->gap(); this->out << "body" << ": "; this->visit(data.body);
				CLOSE
			}

			//|------|
			//| stmt |
			//|------|

			void visit(const lang::$if& data) override
			{
				START
				this->gap(); this->out << "[if]" << "\n";
				this->gap(); this->out << "block" << ": "; this->visit(data.block);
				this->gap(); this->out << "cases" << ": "; this->visit(data.cases);
				CLOSE
			}

			void visit(const lang::$match& data) override
			{
				START
				this->gap(); this->out << "[match]" << "\n";
				this->gap(); this->out << "input" << ": "; this->visit(data.input);
				this->gap(); this->out << "block" << ": "; this->visit(data.block);
				this->gap(); this->out << "cases" << ": "; this->visit(data.cases);
				CLOSE
			}

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

			void visit(const lang::$while& data) override
			{
				START
				this->gap(); this->out << "[while]" << "\n";
				this->gap(); this->out << "input" << ": "; this->visit(data.input);
				this->gap(); this->out << "block" << ": "; this->visit(data.block);
				CLOSE
			}

			void visit(const lang::$break& data) override
			{
				START
				this->gap(); this->out << "[break]" << "\n";
				this->gap(); this->out << "label" << ": " << data.label << "\n";
				CLOSE
			}

			void visit(const lang::$return& data) override
			{
				START
				this->gap(); this->out << "[return]" << "\n";
				this->gap(); this->out << "value" << ": "; this->visit(data.value);
				CLOSE
			}

			void visit(const lang::$continue& data) override
			{
				START
				this->gap(); this->out << "[continue]" << "\n";
				this->gap(); this->out << "label" << ": " << data.label << "\n";
				CLOSE
			}

			//|------|
			//| expr |
			//|------|

			void visit(const lang::$unary& data) override
			{
				START
				this->gap(); this->out << "[unary]" << "\n";
				this->gap(); this->out << "lhs" << ": " << data.lhs << "\n";
				this->gap(); this->out << "rhs" << ": "; this->visit(data.rhs);
				CLOSE
			}

			void visit(const lang::$binary& data) override
			{
				START
				this->gap(); this->out << "[binary]" << "\n";
				this->gap(); this->out << "lhs" << ": "; this->visit(data.lhs);
				this->gap(); this->out << "mhs" << ": " << data.mhs << "\n";
				this->gap(); this->out << "rhs" << ": "; this->visit(data.rhs);
				CLOSE
			}

			void visit(const lang::$literal& data) override
			{
				START
				this->gap(); this->out << "[literal]" << "\n";
				this->gap(); this->out << "type" << ": " << data.type << "\n";
				this->gap(); this->out << "data" << ": " << data.data << "\n";
				CLOSE
			}

			void visit(const lang::$symbol& data) override
			{

				START
				this->gap(); this->out << "[symbol]" << "\n";
				this->gap(); this->out << "name" << ": " << data.name << "\n";
				CLOSE
			}

			void visit(const lang::$group& data) override
			{
				START
				this->gap(); this->out << "[group]" << "\n";
				this->gap(); this->out << "expr" << ": "; this->visit(data.expr);
				CLOSE
			}

			void visit(const lang::$call& data) override
			{
				START
				this->gap(); this->out << "[call]" << "\n";
				this->gap(); this->out << "type" << ": " << data.name << "\n";
				this->gap(); this->out << "name" << ": " << data.type << "\n";
				this->gap(); this->out << "args" << ": "; this->visit(data.args);
				CLOSE
			}

			#undef START
			#undef CLOSE
		}
		printer {std::cout};

		for (const auto& node : this->ast)
		{
			std::visit([&](const auto& arg)
			{
				printer.visit(arg);
			},
			node);
		}
	}
};
