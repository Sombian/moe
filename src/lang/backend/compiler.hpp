#pragma once

#include <map>
#include <vector>
#include <memory>
#include <ranges>
#include <variant>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <fstream>

#include "models/str.hpp"

#include "lang/common/ast.hpp"

#include "traits/rule_of_5.hpp"
#include "traits/visitable.hpp"

                  /**\----------------------------\**/
#define only(...) /**/        __VA_ARGS__         /**/
#define many(...) /**/  std::vector<__VA_ARGS__>  /**/
#define some(...) /**/ std::optional<__VA_ARGS__> /**/
                  /**\----------------------------\**/

class compiler
{
	struct REG
	{
		utf8 a;
		utf8 b;
		utf8 c;
		utf8 d;
	};

	struct INS
	{
		utf8 mov;
		utf8 add;
		utf8 sub;
		utf8 mul;
		utf8 div;
		utf8 cmp;
	};

	struct memory;
	struct typing;

	// fuck MSVC
	struct frame
	{
		size_t stack {0};

		COPY_CONSTRUCTOR(frame) = delete;
		MOVE_CONSTRUCTOR(frame) = default;
		
		frame() = default;
		
		COPY_ASSIGNMENT(frame) = delete;
		MOVE_ASSIGNMENT(frame) = default;

		std::map<utf8, std::unique_ptr<typing>> typing;
		std::map<utf8, std::unique_ptr<memory>> memory;
	};

	struct typing
	{
		const REG* reg;
		const INS* ins;

		typing
		(
			decltype(reg) reg,
			decltype(ins) ins
		)
		: reg {reg}, ins {ins} {}

		virtual ~typing() = default;

		virtual constexpr auto align() const -> size_t = 0;
		virtual constexpr auto bytes() const -> size_t = 0;
	};

	struct type_prime : public typing
	{
		only(uint8_t) width;

		type_prime
		(
			decltype(reg) reg,
			decltype(ins) ins,
			decltype(width) width
		)
		: typing {reg, ins}, width {width} {}

		inline constexpr auto align() const -> size_t override
		{
			return 69;
		};
		
		inline constexpr auto bytes() const -> size_t override
		{
			return 69;
		};
	};

	struct type_model : public typing
	{
		many(typing*) entry;

		type_model
		(
			// decltype(reg) reg,
			// decltype(ins) ins,
			decltype(entry) entry
		)
		: typing {nullptr, nullptr}, entry {entry} {}

		inline constexpr auto align() const -> size_t override
		{
			return 69;
		};

		inline constexpr auto bytes() const -> size_t override
		{
			return 69;
		};
	};

	struct memory
	{
		typing* layout;

		virtual ~memory() = default;

		memory
		(
			typing* layout
		)
		: layout {layout} {}
		
		virtual /*Ი︵𐑼*/ auto point() const -> utf8 = 0;
		virtual /*Ი︵𐑼*/ auto deref() const -> utf8 = 0;
	};

	struct memo_store : public memory
	{
		utf8 symbol;

		memo_store
		(
			decltype(layout) layout,
			decltype(symbol) symbol
		)
		: memory {layout}, symbol {symbol} {}

		inline /*Ი︵𐑼*/ auto point() const -> utf8 override { return u8" %s "_utf | this->symbol; }
		inline /*Ი︵𐑼*/ auto deref() const -> utf8 override { return u8"[%s]"_utf | this->symbol; }
	};

	struct memo_local : public memory
	{
		size_t offset;

		memo_local
		(
			decltype(layout) layout,
			decltype(offset) offset
		)
		: memory {layout}, offset {offset} {}

		inline /*Ი︵𐑼*/ auto point() const -> utf8 override { return u8"[rsp + %s]"_utf | this->offset; }
		inline /*Ი︵𐑼*/ auto deref() const -> utf8 override { return u8"[rsp + %s]"_utf | this->offset; }
	};

	/**\---------------------------\**/
	/**/     size_t label {0};     /**/
	/**\---------------------------\**/
	/**/ std::vector<frame> scope; /**/
	/**\---------------------------\**/

public:

	template
	<
		class A,
		class B
	>
	inline constexpr auto compile(program<A, B>& exe)
	{
		              /**\-----------------------------\**/
		#define ENTER /**/ this->scope.emplace_back(); /**/
		#define LEAVE /**/   this->scope.pop_back();   /**/
		              /**\-----------------------------\**/

		utf8 _init_;
		utf8 _data_;
		utf8 _text_;

		              /**\--------------------\**/
		#define local /**/ this->scope.back() /**/
		              /**\--------------------\**/

		ENTER
		{
			//|--------------|
			//| STEP 1. decl |
			//|--------------|

			// register sets
			static const REG GPR
			{
				.a {u8"rax"},
				.b {u8"rbx"},
				.c {u8"rcx"},
				.d {u8"rdx"},
			};
			// register sets
			static const REG FPR
			{
				.a {u8"xmm0"},
				.b {u8"xmm1"},
				.c {u8"xmm2"},
				.d {u8"xmm3"},
			};
			// instruction sets
			static const INS I64
			{
				.mov {u8"mov" },
				.add {u8"add" },
				.sub {u8"sub" },
				.mul {u8"imul"},
				.div {u8"idiv"},
				.cmp {u8"cmp" },
			};
			// instruction sets
			static const INS U64
			{
				.mov {u8"mov" },
				.add {u8"add" },
				.sub {u8"sub" },
				.mul {u8"mul" },
				.div {u8"div" },
				.cmp {u8"cmp" },
			};
			// instruction sets
			static const INS F32
			{
				.mov {u8"movss"  },
				.add {u8"addss"  },
				.sub {u8"subss"  },
				.mul {u8"mulss"  },
				.div {u8"divss"  },
				.cmp {u8"ucomiss"},
			};
			// instruction sets
			static const INS F64
			{
				.mov {u8"movsd"  },
				.add {u8"addsd"  },
				.sub {u8"subsd"  },
				.mul {u8"mulsd"  },
				.div {u8"divsd"  },
				.cmp {u8"ucomisd"},
			};

			// fundamental type (signed)
			local.typing[u8"i8" ] = std::make_unique<type_prime>(&GPR, &I64, 1 * 8);
			local.typing[u8"i16"] = std::make_unique<type_prime>(&GPR, &I64, 2 * 8);
			local.typing[u8"i32"] = std::make_unique<type_prime>(&GPR, &I64, 4 * 8);
			local.typing[u8"i64"] = std::make_unique<type_prime>(&GPR, &I64, 8 * 8);

			// fundamental type (floating)
			local.typing[u8"f32"] = std::make_unique<type_prime>(&FPR, &F32, 2 * 8);
			local.typing[u8"f64"] = std::make_unique<type_prime>(&FPR, &F64, 4 * 8);

			// fundamental type (unsigned)
			local.typing[u8"u8" ] = std::make_unique<type_prime>(&GPR, &U64, 1 * 8);
			local.typing[u8"u16"] = std::make_unique<type_prime>(&GPR, &U64, 2 * 8);
			local.typing[u8"u32"] = std::make_unique<type_prime>(&GPR, &U64, 4 * 8);
			local.typing[u8"u64"] = std::make_unique<type_prime>(&GPR, &U64, 8 * 8);

			// fundamental type (trivial)
			local.typing[u8"none"] = std::make_unique<type_prime>(&GPR, &I64, 0 * 8);
			local.typing[u8"bool"] = std::make_unique<type_prime>(&GPR, &I64, 1 * 8);
			local.typing[u8"word"] = std::make_unique<type_prime>(&GPR, &I64, 4 * 8);

			for (auto& _ : exe.body)
			{
				std::visit(fix{visitor<void>
				(
					// notice: scan top-level models only for now
					[&](auto& self, std::unique_ptr<model_decl>& decl)
					{
						local.typing[decl->name] = std::make_unique<type_model>([&]
						{
							// pre-allocate by size of body
							decltype(type_model::entry) impl
							/*----------------------------*/
							{      decl->body.size()      };
							/*----------------------------*/

							for (size_t i {0}; i < decl->body.size(); ++i)
							{
								// use of subscript op's insertion side effect
								impl[i] = local.typing[decl->body[i].type].get();
								// also get the raw ptr out of std::unique_ptr
							} 
							return impl; // RVO -> so we dont need to std::move
						}
						());
					}
				)},
				_);
			}

			//|-----------------|
			//| STEP 2. codegen |
			//|-----------------|

			for (auto& _ : exe.body)
			{
				std::visit(fix{visitor<void>
				(
					// variant::decl
					[&](auto& self, std::unique_ptr<var_decl>& decl)
					{

					},
					[&](auto& self, std::unique_ptr<fun_decl>& decl)
					{

					},
					[&](auto& self, std::unique_ptr<model_decl>& decl)
					{

					},
					[&](auto& self, std::unique_ptr<trait_decl>& decl)
					{

					},
					// variant::stmt
					[&](auto& self, std::unique_ptr<if_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<for_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<match_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<while_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<block_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<break_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<return_stmt>& stmt)
					{

					},
					[&](auto& self, std::unique_ptr<iterate_stmt>& stmt)
					{

					},
					// variant::expr
					[&](auto&& self, std::unique_ptr<prefix_expr>& expr)
					{

					},
					[&](auto& self, std::unique_ptr<binary_expr>& expr)
					{
						
					},
					[&](auto& self, std::unique_ptr<suffix_expr>& expr)
					{

					},
					[&](auto& self, std::unique_ptr<access_expr>& expr)
					{

					},
					[&](auto& self, std::unique_ptr<invoke_expr>& expr)
					{

					},
					[&](auto& self, std::unique_ptr<literal_expr>& expr)
					{

					},
					[&](auto& self, std::unique_ptr<symbol_expr>& expr)
					{

					},
					[&](auto& self, std::unique_ptr<group_expr>& expr)
					{

					}
				)},
				_);
			}

			//|-------------------|
			//| STEP 3. fs::write |
			//|-------------------|

			if (std::ofstream ofs {"main.asm"})
			{
				ofs << "bits 64"       << '\n';
				ofs << "default rel"   << '\n';
				ofs <<                    '\n';
				ofs << "section .bss"  << '\n';
				ofs << "; variables"   << '\n';
				ofs << _init_;
				ofs <<                    '\n';
				ofs << "section .data" << '\n';
				ofs << "; constants"   << '\n';
				ofs << _data_;
				ofs <<                    '\n';
				ofs << "section .text" << '\n';
				ofs << "global main"   << '\n';
				ofs <<                    '\n';
				ofs << _text_;
			}
		}
		LEAVE

		#undef ENTER
		#undef LEAVE
		#undef local
	}

private:

	//|-----------------|
	//| resolve::memory |
	//|-----------------|

	inline /*Ი︵𐑼*/ auto resolve_var(model::text auto& name) -> memory*
	{
		for (auto& _ : std::ranges::reverse_view(this->scope))
		{
			if (_.memory.contains(name))
			{
				return _.memory[name].get();
			}
		}
		assert(false);
		return nullptr;
	}

	template<size_t N>
	// converting constructor
	inline /*Ი︵𐑼*/ auto resolve_var(const char8_t (&name)[N]) -> memory*
	{
		const utf8 lvalue {name}; return this->resolve_var(lvalue);
	}

	template<size_t N>
	// converting constructor
	inline /*Ი︵𐑼*/ auto resolve_var(const char16_t (&name)[N]) -> memory*
	{
		const utf16 lvalue {name}; return this->resolve_var(lvalue);
	}

	template<size_t N>
	// converting constructor
	inline /*Ი︵𐑼*/ auto resolve_var(const char32_t (&name)[N]) -> memory*
	{
		const utf32 lvalue {name}; return this->resolve_var(lvalue);
	}

	//|-----------------|
	//| resolve::typing |
	//|-----------------|

	inline /*Ი︵𐑼*/ auto resolve_type(model::text auto& name) -> typing*
	{
		for (auto& _ : std::ranges::reverse_view(this->scope))
		{
			if (_.typing.contains(name))
			{
				return _.typing[name].get();
			}
		}
		assert(false);
		return nullptr;
	}

	template<size_t N>
	// converting constructor
	inline /*Ი︵𐑼*/ auto resolve_type(const char8_t (&name)[N]) -> typing*
	{
		const utf8 lvalue {name}; return this->resolve_type(lvalue);
	}

	template<size_t N>
	// converting constructor
	inline /*Ი︵𐑼*/ auto resolve_type(const char16_t (&name)[N]) -> typing*
	{
		const utf16 lvalue {name}; return this->resolve_type(lvalue);
	}

	template<size_t N>
	// converting constructor
	inline /*Ი︵𐑼*/ auto resolve_type(const char32_t (&name)[N]) -> typing*
	{
		const utf32 lvalue {name}; return this->resolve_type(lvalue);
	}

	//|----------------|
	//| type inference |
	//|----------------|

	inline /*Ი︵𐑼*/ auto infer_type(expr& expr) -> typing*
	{
		return std::visit(fix{visitor<typing*>
		(
			[&](auto& self, std::unique_ptr<literal_expr>& expr)
			{
				switch (expr->type)
				{
					case ty::I8:  return this->resolve_type(u8"i8" );
					case ty::I16: return this->resolve_type(u8"i16");
					case ty::I32: return this->resolve_type(u8"i32");
					case ty::I64: return this->resolve_type(u8"i64");

					case ty::F32: return this->resolve_type(u8"f32");
					case ty::F64: return this->resolve_type(u8"f64");

					case ty::U8:  return this->resolve_type(u8"u8") ;
					case ty::U16: return this->resolve_type(u8"u16");
					case ty::U32: return this->resolve_type(u8"u32");
					case ty::U64: return this->resolve_type(u8"u64");

					case ty::BOOL: return this->resolve_type(u8"bool");
					case ty::WORD: return this->resolve_type(u8"word");
					case ty::NONE: return this->resolve_type(u8"none");
				}
				assert(!"<ERROR>");
				std::unreachable();
			},
			[&](auto& self, std::unique_ptr<symbol_expr>& expr)
			{
				return this->resolve_var(expr->self)->layout;
			},
			[&](auto& self, std::unique_ptr<group_expr>& expr)
			{
				return std::visit(self, expr->self);
			},
			[&](auto& self, std::unique_ptr<binary_expr>& expr)
			{
				return std::visit(self, expr->lhs);
			},
			[&](auto& self, std::unique_ptr<prefix_expr>& expr)
			{
				return std::visit(self, expr->rhs);
			},
			[&](auto& self, std::unique_ptr<suffix_expr>& expr)
			{	
				return std::visit(self, expr->lhs);
			},
			[&](auto& self, auto& /* fallback */)
			{
				return nullptr;
			}
		)},
		expr);
	}
};

#undef only
#undef many
#undef some
