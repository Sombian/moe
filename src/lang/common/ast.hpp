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
		virtual void visit(const $unary_l&) = 0;
		virtual void visit(const $binary&) = 0;
		virtual void visit(const $unary_r&) = 0;
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
	struct $unary_l
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

	struct $unary_r
	{
		only<expr> lhs;
		only<op_r> rhs;

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
		only<op_r> type;
		only<utf8> name;
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
		struct printer final : lang::visitor
		{
			std::ostream& out {std::cout};

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
				this->out << "(var";
				this->out << " ";
				this->out << data.name;
				this->out << " ";
				this->out << data.type;
				this->out << " ";
				this->visit(data.init);
				this->out << ")";
			}

			void visit(const lang::$fun& data) override
			{
				this->out << "(fun";
				this->out << " ";
				this->out << data.name;
				this->out << " ";
				this->out << data.type;
				this->out << " ";
				this->visit(data.body);
				this->out << ")";
			}

			//|------|
			//| stmt |
			//|------|

			void visit(const lang::$if& data) override
			{
				this->out << "(if";
				this->out << " ";
				this->visit(data.block);
				this->out << " ";
				this->visit(data.cases);
				this->out << ")";
			}

			void visit(const lang::$match& data) override
			{
				this->out << "(match";
				this->out << " ";
				this->visit(data.input);
				this->out << " ";
				this->visit(data.block);
				this->out << " ";
				this->visit(data.cases);
				this->out << ")";
			}

			void visit(const lang::$for& data) override
			{
				this->out << "(for";
				this->out << " ";
				this->visit(data.setup);
				this->out << " ";
				this->visit(data.input);
				this->out << " ";
				this->visit(data.after);
				this->out << " ";
				this->visit(data.block);
				this->out << ")";
			}

			void visit(const lang::$while& data) override
			{
				this->out << "(while";
				this->out << " ";
				this->visit(data.input);
				this->out << " ";
				this->visit(data.block);
				this->out << ")";
			}

			void visit(const lang::$break& data) override
			{
				this->out << "(break";
				this->out << " ";
				this->out << data.label;
				this->out << ")";
			}

			void visit(const lang::$return& data) override
			{
				this->out << "(return";
				this->out << " ";
				this->visit(data.value);
				this->out << ")";
			}

			void visit(const lang::$continue& data) override
			{
				this->out << "(continue";
				this->out << " ";
				this->out << data.label;
				this->out << ")";
			}

			//|------|
			//| expr |
			//|------|

			void visit(const lang::$unary_l& data) override
			{
				this->out << "(unary_l";
				this->out << " ";
				this->out << data.lhs;
				this->out << " ";
				this->visit(data.rhs);
				this->out << ")";
			}

			void visit(const lang::$binary& data) override
			{
				this->out << "(binary";
				this->out << " ";
				this->visit(data.lhs);
				this->out << " ";
				this->out << data.mhs;
				this->out << " ";
				this->visit(data.rhs);
				this->out << ")";
			}

			void visit(const lang::$unary_r& data) override
			{
				this->out << "(unary_r";
				this->out << " ";
				this->visit(data.lhs);
				this->out << " ";
				this->out << data.rhs;
				this->out << ")";
			}

			void visit(const lang::$literal& data) override
			{
				this->out << "(literal";
				this->out << " ";
				this->out << data.type;
				this->out << " ";
				this->out << data.data;
				this->out << ")";
			}

			void visit(const lang::$symbol& data) override
			{
				this->out << "(symbol";
				this->out << " ";
				this->out << data.name;
				this->out << ")";
			}

			void visit(const lang::$group& data) override
			{
				this->out << "(group";
				this->out << " ";
				this->visit(data.expr);
				this->out << ")";
			}

			void visit(const lang::$call& data) override
			{
				this->out << "(call";
				this->out << " ";
				this->out << data.type;
				this->out << " ";
				this->out << data.name;
				this->out << " ";
				this->visit(data.args);
				this->out << ")";
			}
		}
		printer;

		for (const auto& node : this->ast)
		{
			std::visit([&](const auto& arg)
			{
				printer.visit(arg);
				std::cout << "\n";
			},
			node);
		}
	}
};
