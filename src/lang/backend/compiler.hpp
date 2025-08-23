#pragma once

#include <map>
#include <memory>
#include <vector>
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
	struct ins_t
	{
		const char8_t* add;
		const char8_t* sub;
		const char8_t* mul;
		const char8_t* div;
	};

	struct memory;
	struct typing;

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

	struct memory
	{
		size_t offset;
		typing* layout;

		memory
		(
			size_t offset,
			typing* layout
		)
		: offset {offset}, layout {layout} {}
		
		inline /*Ი︵𐑼*/ auto point() const -> utf8
		{
			switch (this->layout->bytes())
			{
				case 1: return u8"[rbp - %s]"_utf | this->offset;
				case 2: return u8"[rbp - %s]"_utf | this->offset;
				case 4: return u8"[rbp - %s]"_utf | this->offset;
				case 8: return u8"[rbp - %s]"_utf | this->offset;
			}
		}
		inline /*Ი︵𐑼*/ auto deref() const -> utf8
		{
			switch (this->layout->bytes())
			{
				case 1: return  u8"BYTE PTR [rbp - %s]"_utf | this->offset;
				case 2: return  u8"WORD PTR [rbp - %s]"_utf | this->offset;
				case 4: return u8"DWORD PTR [rbp - %s]"_utf | this->offset;
				case 8: return u8"QWORD PTR [rbp - %s]"_utf | this->offset;
			}
		}
	};

	struct typing
	{
		virtual ~typing() = default;

		virtual constexpr auto align() const -> size_t = 0;
		virtual constexpr auto bytes() const -> size_t = 0;
	};

	struct prime_t : public typing
	{
		const ins_t* ins;
		const only(uint8_t) width;

		prime_t
		(
			decltype(ins) ins,
			decltype(width) width
		)
		: ins {ins}, width {width} {}

	public:

		inline constexpr auto align() const -> size_t override
		{
			return this->width / 8;
		}
		
		inline constexpr auto bytes() const -> size_t override
		{
			return this->width / 8;
		}
	};

	struct model_t : public typing
	{
		const bool pkg;
		const many(typing*) entry;

		model_t
		(
			decltype(pkg) pkg,
			decltype(entry) width
		)
		: pkg {pkg}, entry {entry} {}

	public:

		inline constexpr auto align() const -> size_t override
		{
			return 69;
		}

		inline constexpr auto bytes() const -> size_t override
		{
			return 69;
		}
	};

	/**\---------------------------\**/
	/**/       utf8 program;       /**/
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
	inline constexpr auto compile(AST<A, B>& exe)
	{
		              /**\-----------------------------\**/
		#define ENTER /**/ this->scope.emplace_back(); /**/
		#define LEAVE /**/   this->scope.pop_back();   /**/
		              /**\-----------------------------\**/

		              /**\--------------------\**/
		#define local /**/ this->scope.back() /**/
		              /**\--------------------\**/

		ENTER
		{
			//|--------------|
			//| STEP 1. decl |
			//|--------------|

			// fundamental type (signed)
			local.typing[u8"i8" ] = std::make_unique<prime_t>(&I64, 1 * 8);
			local.typing[u8"i16"] = std::make_unique<prime_t>(&I64, 2 * 8);
			local.typing[u8"i32"] = std::make_unique<prime_t>(&I64, 4 * 8);
			local.typing[u8"i64"] = std::make_unique<prime_t>(&I64, 8 * 8);

			// fundamental type (floating)
			local.typing[u8"f32"] = std::make_unique<prime_t>(&F32, 2 * 8);
			local.typing[u8"f64"] = std::make_unique<prime_t>(&F64, 4 * 8);

			// fundamental type (unsigned)
			local.typing[u8"u8" ] = std::make_unique<prime_t>(&U64, 1 * 8);
			local.typing[u8"u16"] = std::make_unique<prime_t>(&U64, 2 * 8);
			local.typing[u8"u32"] = std::make_unique<prime_t>(&U64, 4 * 8);
			local.typing[u8"u64"] = std::make_unique<prime_t>(&U64, 8 * 8);

			// fundamental type (trivial)
			local.typing[u8"none"] = std::make_unique<prime_t>(&I64, 0 * 8);
			local.typing[u8"bool"] = std::make_unique<prime_t>(&I64, 1 * 8);
			local.typing[u8"word"] = std::make_unique<prime_t>(&I64, 4 * 8);

			for (auto& node : exe.body)
			{
				std::visit(fix{visitor<void>
				(
					// notice: scan top-level models only for now
					[&](auto& self, std::unique_ptr<model_decl>& decl)
					{
						// pre-define struct as its expected for modern lang
						local.typing[decl->name] = std::make_unique<model_t>(
						// do not tightly pack by default, and init struct fields
						false, [&]
						{
							// make just enough room
							std::vector<typing*> impl
							/*----------------------*/
							{   decl->body.size()   };
							/*----------------------*/

							for (size_t i {0}; i < decl->body.size(); ++i)
							{
								// use of subscript op's insertion side effect
								impl[i] = local.typing[decl->body[i].type].get();
								// also get the raw ptr out of std::unique_ptr
							} 
							return impl; // RVO, so we dont need to std::move
						}());
					}
				)},
				node);
			}

			//|-----------------|
			//| STEP 2. codegen |
			//|-----------------|

			for (auto& node : exe.body)
			{
				std::visit(fix{visitor<void>
				(
					// variant::decl
					[&](auto& self, std::unique_ptr<var_decl>& decl)
					{
						auto* type {this->resolve_type(decl->type)};

						static auto unwrap
						{
							[&](expr& expr) -> literal_expr*
							{
								return std::visit(fix{visitor<literal_expr*>
								(
									[&](auto& self, std::unique_ptr<literal_expr>& expr)
									{
										return expr.get();
									},
									[&](auto& self, std::unique_ptr<group_expr>& expr)
									{
										return std::visit(self, expr->self);
									},
									[&](auto& self, auto& /* fallback for other expr */)
									{
										return nullptr;
									}
								)},
								expr);
							}
						};
						
						local.memory[decl->name] = std::make_unique<memory>(local.stack += type->bytes(), type);

						// decrease stack pointer by sizeof(variable)
						this->program += u8"\tsub rsp, %s\n"_utf | type->bytes();

						if (auto& opt {decl->init})
						{
							// immediate value
							if (auto* lit {unwrap(*opt)})
							{
								// TODO: convert floating points immediate to integral representation
								this->program += u8"\tmov %s, %s\n"_utf | local.memory[decl->name]->deref() | lit->self;
							}
							else // complex expr
							{
								// TODO: feeds expr into a expr exclusive visitor then get the register
								this->program += u8"\tmov %s, %s\n"_utf | local.memory[decl->name]->deref() /* register */;
							}
						}
					},
					[&](auto& self, std::unique_ptr<fun_decl>& decl)
					{
						// label
						this->program += u8"%s:\n"_utf | decl->name;

						// prologue
						this->program += u8"\tsub rsp, 0x8\n"_utf;
						this->program += u8"\tmov [rsp], rbp\n"_utf;
						this->program += u8"\tmov rbp, rsp\n"_utf;

						ENTER
						{
							for (auto& node : decl->body)
							{
								std::visit(self, node);
							}
						}
						LEAVE

						// epilogue
						this->program += u8"\tmov rsp, rbp\n"_utf;
						this->program += u8"\tmov rbp, [rsp]\n"_utf;
						this->program += u8"\tadd rsp, 0x8\n"_utf;

						// exit
						this->program += u8"\tret"_utf;
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
				node);
			}

			//|-------------------|
			//| STEP 3. fs::write |
			//|-------------------|

			if (std::ofstream ofs {"main.asm"})
			{
				ofs << "bits 64"       << '\n';
				ofs << "default rel"   << '\n';
				ofs <<                    '\n';
				ofs << "section .text" << '\n';
				ofs << "global _start" << '\n';
				ofs <<  this->program  << '\n';
			}
		}
		LEAVE

		#undef ENTER
		#undef LEAVE
		#undef local
	}

private:

	bool fGPR[4] {true};
	bool fFPR[4] {true};

	constexpr static const
	char8_t* const GPR[]
	{
		{u8"rax"},
		{u8"rbx"},
		{u8"rcx"},
		{u8"rdx"},
	};
	constexpr static const
	char8_t* const FPR[]
	{
		{u8"xmm0"},
		{u8"xmm1"},
		{u8"xmm2"},
		{u8"xmm3"},
	};

	constexpr static
	const ins_t I64
	{
		.add {u8"add" },
		.sub {u8"sub" },
		.mul {u8"imul"},
		.div {u8"idiv"},
	};

	constexpr static
	const ins_t U64
	{
		.add {u8"add" },
		.sub {u8"sub" },
		.mul {u8"mul" },
		.div {u8"div" },
	};

	constexpr static
	const ins_t F32
	{
		.add {u8"addss"},
		.sub {u8"subss"},
		.mul {u8"mulss"},
		.div {u8"divss"},
	};

	constexpr static
	const ins_t F64
	{
		.add {u8"addsd"},
		.sub {u8"subsd"},
		.mul {u8"mulsd"},
		.div {u8"divsd"},
	};

	//|----------------|
	//| asm::registers |
	//|----------------|

	// free every GPR
	inline constexpr auto free_gpr()
	{
		fGPR[0] = true;
		fGPR[1] = true;
		fGPR[2] = true;
		fGPR[3] = true;
	}

	// free every FPR
	inline constexpr auto free_fpr()
	{
		fFPR[0] = true;
		fFPR[1] = true;
		fFPR[2] = true;
		fFPR[3] = true;
	}

	// free a GPR at given index
	inline constexpr auto free_gpr(short i)
	{
		if (!GPR[i])
		{
			// double free error
			assert(!"<ERROR>");
			std::unreachable();
		}
		fGPR[i] = true;
	}

	// free a FPR at given index
	inline constexpr auto free_fpr(short i)
	{
		if (!FPR[i])
		{
			// double free error
			assert(!"<ERROR>");
			std::unreachable();
		}
		fFPR[i] = true;
	}

	// returns an index to the available GPR
	inline constexpr auto pull_gpr() -> short
	{
		short i {0};

		for (; i < 4; ++i)
		{
			if (fGPR[i])
			{
				fGPR[i] = false;
				return i; // slot
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	// returns an index to the available FPR
	inline constexpr auto pull_fpr() -> short
	{
		short i {0};
		
		for (; i < 4; ++i)
		{
			if (fFPR[i])
			{
				fFPR[i] = false;
				return i; // slot
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	//|-----------------|
	//| asm::arithmetic |
	//|-----------------|

	// add r1, r2 => r2 += r1
	inline constexpr auto cg_add(ins_t* i1, short r1, ins_t* i2, short r2) -> short
	{
		if (i1 == i2)
		{
			if (i1 == &I64 || i1 == &U64)
			{
				this->program += u8"%s %s, %s"_utf | i1->add | GPR[r1] | GPR[r2];
				
				free_gpr(r1); return r2;
			}
			if (i1 == &F32 || i1 == &F64)
			{
				this->program += u8"%s %s, %s"_utf | i1->add | FPR[r1] | FPR[r2];
				
				free_fpr(r1); return r2;
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	// sub r1, r2 => r2 -= r1
	inline constexpr auto cg_sub(ins_t* i1, short r1, ins_t* i2, short r2) -> short
	{
		if (i1 == i2)
		{
			if (i1 == &I64 || i1 == &U64)
			{
				this->program += u8"%s %s, %s"_utf | i1->sub | GPR[r1] | GPR[r2];
				
				free_gpr(r1); return r2;
			}
			if (i1 == &F32 || i1 == &F64)
			{
				this->program += u8"%s %s, %s"_utf | i1->sub | FPR[r1] | FPR[r2];
				
				free_fpr(r1); return r2;
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	// mul r1, r2 => r2 *= r1
	inline constexpr auto cg_mul(ins_t* i1, short r1, ins_t* i2, short r2) -> short
	{
		if (i1 == i2)
		{
			if (i1 == &I64 || i1 == &U64)
			{
				this->program += u8"%s %s, %s"_utf | i1->mul | GPR[r1] | GPR[r2];
				
				free_gpr(r1); return r2;
			}
			if (i1 == &F32 || i1 == &F64)
			{
				this->program += u8"%s %s, %s"_utf | i1->mul | FPR[r1] | FPR[r2];
				
				free_fpr(r1); return r2;
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	inline constexpr auto cg_div(ins_t* i1, short r1, ins_t* i2, short r2) -> short
	{
		if (i1 == i2)
		{
			if (i1 == &I64 || i1 == &U64)
			{
				return 69; // TODO
			}
			if (i1 == &F32 || i1 == &F64)
			{
				return 69; // TODO
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	inline constexpr auto cg_mod(ins_t* i1, short r1, ins_t* i2, short r2) -> short
	{
		if (i1 == i2)
		{
			if (i1 == &I64 || i1 == &U64)
			{
				return 69; // TODO
			}
			if (i1 == &F32 || i1 == &F64)
			{
				return 69; // TODO
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	inline constexpr auto cg_pow(ins_t* i1, short r1, ins_t* i2, short r2) -> short
	{
		if (i1 == i2)
		{
			if (i1 == &I64 || i1 == &U64)
			{
				return 69; // TODO
			}
			if (i1 == &F32 || i1 == &F64)
			{
				return 69; // TODO
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	inline constexpr auto cg_load(int raw) -> short
	{
		auto r1 {this->pull_gpr()};

		this->program += u8"\tmov %s, %s\n"_utf | GPR[r1] | raw;

		return r1;
	}

	inline constexpr auto cg_load(long raw) -> short
	{
		auto r1 {this->pull_gpr()};

		this->program += u8"\tmov %s, %s\n"_utf | GPR[r1] | raw;

		return r1;
	}

	inline constexpr auto cg_load(float raw) -> short
	{
		auto r1 {this->pull_fpr()};

		this->program += u8"\tmovss %s, %s\n"_utf | FPR[r1] | raw;

		return r1;
	}

	inline constexpr auto cg_load(double raw) -> short
	{
		auto r1 {this->pull_fpr()};

		this->program += u8"\tmovsd %s, %s\n"_utf | FPR[r1] | raw;

		return r1;
	}

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
			[&](auto& self, auto& /* fallback for other expr */)
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
