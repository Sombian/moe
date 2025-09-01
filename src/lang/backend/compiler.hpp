#pragma once

#include <map>
#include <memory>
#include <vector>
#include <ranges>
#include <variant>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <fstream>

#include "models/str.hpp"

#include "lang/common/ast.hpp"

#include "traits/rule_of_5.hpp"
#include "traits/visitable.hpp"

#include "utils/convert.hpp"

                  /**\----------------------------\**/
#define only(...) /**/        __VA_ARGS__         /**/
#define many(...) /**/  std::vector<__VA_ARGS__>  /**/
#define some(...) /**/ std::optional<__VA_ARGS__> /**/
                  /**\----------------------------\**/

class compiler
{
	struct ins_t
	{
		const char8_t* mov;
		const char8_t* add;
		const char8_t* sub;
		const char8_t* mul;
		const char8_t* div;
	};

	struct reg_t
	{
		std::pair<utf8, bool>* bank;
		uint8_t                slot;

		constexpr operator utf8&() const
		{
			return this->bank[this->slot].first;
		}

		inline constexpr auto is_free() const -> bool
		{
			return this->bank[this->slot].second;
		}

		inline constexpr auto release() const -> void
		{
			this->bank[this->slot].second = true;
		}
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
			assert(!"<ERROR>");
			std::unreachable();
		}
		inline /*Ი︵𐑼*/ auto deref() const -> utf8
		{
			switch (this->layout->bytes())
			{
				case 1: return  u8"BYTE [rbp - %s]"_utf | this->offset;
				case 2: return  u8"WORD [rbp - %s]"_utf | this->offset;
				case 4: return u8"DWORD [rbp - %s]"_utf | this->offset;
				case 8: return u8"QWORD [rbp - %s]"_utf | this->offset;
			}
			assert(!"<ERROR>");
			std::unreachable();
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
			decltype(entry) entry
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
			local.typing[u8"f32"] = std::make_unique<prime_t>(&F32, 4 * 8);
			local.typing[u8"f64"] = std::make_unique<prime_t>(&F64, 8 * 8);

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
						if (auto* type {this->resolve_type(decl->type)})
						{
							// TODO: 16 byte alignment
							auto align {type->bytes()};

							//|----------------------------------------------------------------------------|
							local.memory[decl->name] = std::make_unique<memory>(local.stack += align, type);
							//|----------------------------------------------------------------------------|

							// TODO: initialize struct
							if (decl->init)
							{
								if (auto t1 {dynamic_cast<prime_t*>(type)})
								{
									auto r1 {this->cg(decl->init.value())};

									if (t1->ins == &I64 || t1->ins == &U64)
									{
										//|------------------------------------------------------------------------------------------------------------------------|
										this->program += u8"\t%s %s, %s\n"_utf | t1->ins->mov | local.memory[decl->name]->deref() | this->view_gpr(r1, t1->bytes());
										//|------------------------------------------------------------------------------------------------------------------------|
									}
									if (t1->ins == &F32 || t1->ins == &F64)
									{
										//|------------------------------------------------------------------------------------------------------------------------|
										this->program += u8"\t%s %s, %s\n"_utf | t1->ins->mov | local.memory[decl->name]->deref() | this->view_fpr(r1, t1->bytes());
										//|------------------------------------------------------------------------------------------------------------------------|
									}
									r1.release(); // discard the register
								}
							}
						}
					},
					[&](auto& self, std::unique_ptr<fun_decl>& decl)
					{
						this->program += u8"%s:\n"_utf | decl->name;

						//|---------------<prologue>---------------|
						this->program += u8"\t;-------------;\n"_utf;
						this->program += u8"\tsub rsp, 0x8  ;\n"_utf;
						this->program += u8"\tmov [rsp], rbp;\n"_utf;
						this->program += u8"\tmov rbp, rsp  ;\n"_utf;
						this->program += u8"\t;-------------;\n"_utf;
						//|----------------------------------------|

						ENTER
						{
							for (auto& node : decl->body)
							{
								std::visit(self, node);
							}
						}
						LEAVE

						//|---------------<epilogue>---------------|
						this->program += u8"\t;-------------;\n"_utf;
						this->program += u8"\tmov rsp, rbp  ;\n"_utf;
						this->program += u8"\tmov rbp, [rsp];\n"_utf;
						this->program += u8"\tadd rsp, 0x8  ;\n"_utf;
						this->program += u8"\t;-------------;\n"_utf;
						//|----------------------------------------|

						this->program += u8"\tret"_utf /* exit */;
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
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<binary_expr>& expr)
					{
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<suffix_expr>& expr)
					{
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<access_expr>& expr)
					{
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<invoke_expr>& expr)
					{
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<literal_expr>& expr)
					{
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<symbol_expr>& expr)
					{
						this->cg(expr).release();
					},
					[&](auto& self, std::unique_ptr<group_expr>& expr)
					{
						this->cg(expr).release();
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
				ofs << "global main"   << '\n';
				ofs <<  this->program  << '\n';
			}
		}
		LEAVE

		#undef ENTER
		#undef LEAVE
		#undef local
	}

private:

	template<class T>
	// codegen an expression
	inline constexpr auto cg(T& e) -> reg_t
	{
		if constexpr (std::is_same_v<T, expr>) // break into pieces
		{
			return std::visit([&](auto& e) { return this->cg(e); }, e);
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<prefix_expr>>)
		{
			// TODO
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<binary_expr>>)
		{
			auto* t1 {dynamic_cast<prime_t*>(this->infer_type(e->lhs))};
			auto* t2 {dynamic_cast<prime_t*>(this->infer_type(e->rhs))};

			if (t1 && t2)
			{
				auto* common {this->promote(t1, t2)};

				auto r1 {this->cg(e->lhs)};
				auto r2 {this->cg(e->rhs)};

				r1 = this->convert(common, r1, t1);
				r2 = this->convert(common, r2, t2);

				switch (e->op)
				{
					case op::ADD: return this->cg_add(common, r1, r2);
					case op::SUB: return this->cg_sub(common, r1, r2);
					case op::MUL: return this->cg_mul(common, r1, r2);
					case op::DIV: return this->cg_div(common, r1, r2);
					case op::MOD: return this->cg_mod(common, r1, r2);
					case op::POW: return this->cg_pow(common, r1, r2);
				}
			}
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<suffix_expr>>)
		{
			// TODO
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<access_expr>>)
		{
			// TODO
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<invoke_expr>>)
		{
			// TODO
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<prefix_expr>>)
		{
			// TODO
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<literal_expr>>)
		{
			switch (e->type)
			{
				case ty::I8 : case ty::U8 :
				case ty::I16: case ty::U16:
				case ty::I32: case ty::U32:
				case ty::I64: case ty::U64:
				{
					return this->cg_load(utils::stoi(e->self));
				}
				case ty::F32: case ty::F64:
				{
					return this->cg_load(utils::stof(e->self));
				}
			}
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<symbol_expr>>)
		{
			auto* var {this->resolve_var(e->self)};

			if (auto* t1 {dynamic_cast<prime_t*>(var->layout)})
			{
				if (t1->ins == &I64 || t1->ins == &U64)
				{
					auto r1 {this->pull_gpr()};

					//|----------------------------------------------------------------------|
					this->program += u8"\t%s %s, %s\n"_utf | t1->ins->mov | r1 | var->deref();
					//|----------------------------------------------------------------------|

					return r1; // allocation..!
				}
				if (t1->ins == &F32 || t1->ins == &F64)
				{
					auto r1 {this->pull_fpr()};

					//|----------------------------------------------------------------------|
					this->program += u8"\t%s %s, %s\n"_utf | t1->ins->mov | r1 | var->deref();
					//|----------------------------------------------------------------------|

					return r1; // allocation..!
				}
			}
		}
		if constexpr (std::is_same_v<T, std::unique_ptr<group_expr>>)
		{
			return this->cg(e->self);
		}
		assert(!"<ERROR>");
		std::unreachable();
		return {nullptr, 0};
	}

	inline constexpr auto promote(const prime_t* lhs, const prime_t* rhs) -> const ins_t*
	{
		if (lhs->ins == &F64 || rhs->ins == &F64) return &F64;
		if (lhs->ins == &F32 || rhs->ins == &F32) return &F32;
		if (lhs->ins == &U64 || rhs->ins == &U64) return &U64;
		                                          return &I64;
	}

	inline constexpr auto convert(const ins_t* target, reg_t src, const prime_t* from) -> reg_t
	{
		// int → f64 conversion
		if (target == &F64 && (from->ins == &I64 || from->ins == &U64))
		{
			auto r1 {this->pull_fpr()};

			this->program += u8"\tcvtsi2sd %s, %s\n"_utf | r1 | src;

			src.release(); return r1;
		}
		// int → f32 conversion
		if (target == &F32 && (from->ins == &I64 || from->ins == &U64))
		{
			auto r1 {this->pull_fpr()};

			this->program += u8"\tcvtsi2ss %s, %s\n"_utf | r1 | src;

			src.release(); return r1;
		}
		// f32 → f64 promotion
		if (target == &F64 && (from->ins == &F32 /* f64 exclusive */))
		{
			auto r1 {this->pull_fpr()};

			this->program += u8"\tcvtss2sd %s, %s\n"_utf | r1 | src;

			src.release(); return r1;
		}

		// f64 → f32 demotion
		if (target == &F32 && (from->ins == &F64 /* f32 exclusive */))
		{
			auto r1 {this->pull_fpr()};

			this->program += u8"\tcvtsd2ss %s, %s\n"_utf | r1 | src;

			src.release(); return r1;
		}

		return src;
	}

	std::pair<utf8, bool> GPR[4]
	{
		{u8"rax", true},
		{u8"rbx", true},
		{u8"rcx", true},
		{u8"rdx", true},
	};

	std::pair<utf8, bool> FPR[4]
	{
		{u8"xmm0", true},
		{u8"xmm1", true},
		{u8"xmm2", true},
		{u8"xmm3", true},
	};

	constexpr static const ins_t I64
	{
		.mov {u8"mov" },
		.add {u8"add" },
		.sub {u8"sub" },
		.mul {u8"imul"},
		.div {u8"idiv"},
	};

	constexpr static const ins_t U64
	{
		.mov {u8"mov" },
		.add {u8"add" },
		.sub {u8"sub" },
		.mul {u8"mul" },
		.div {u8"div" },
	};

	constexpr static const ins_t F32
	{
		.mov {u8"movss"},
		.add {u8"addss"},
		.sub {u8"subss"},
		.mul {u8"mulss"},
		.div {u8"divss"},
	};

	constexpr static const ins_t F64
	{
		.mov {u8"movsd"},
		.add {u8"addsd"},
		.sub {u8"subsd"},
		.mul {u8"mulsd"},
		.div {u8"divsd"},
	};

	//|----------------|
	//| asm::registers |
	//|----------------|

	// free every GPR
	inline constexpr void free_gpr()
	{
		GPR[0].second = true;
		GPR[1].second = true;
		GPR[2].second = true;
		GPR[3].second = true;
	}

	// free every FPR
	inline constexpr void free_fpr()
	{
		FPR[0].second = true;
		FPR[1].second = true;
		FPR[2].second = true;
		FPR[3].second = true;
	}

	// returns an index to the available GPR
	inline constexpr auto pull_gpr() -> reg_t
	{
		uint8_t i {0};

		for (; i < 4; ++i)
		{
			if (GPR[i].second)
			{
				GPR[i].second = false;
				return { &GPR[0], i };
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	// returns an index to the available FPR
	inline constexpr auto pull_fpr() -> reg_t
	{
		uint8_t i {0};

		for (; i < 4; ++i)
		{
			if (FPR[i].second)
			{
				FPR[i].second = false;
				return { &FPR[0], i };
			}
		}
		assert(!"<ERROR>");
		std::unreachable();
	}

	inline constexpr auto view_gpr(const reg_t& rg, size_t bytes) -> utf8
	{
		//┌────────┬────────┬─────────────────┬───────────────────────────────────────┐
		//│ 8 bits │ 8 bits │     16 bits     │                 32 bits               │
		//├────────┴────────┴─────────────────┴───────────────────────────────────────┤
		//│                                  rax                                      │
		//├───────────────────────────────────┬───────────────────────────────────────┤
		//│                eax                │                                       │
		//├─────────────────┬─────────────────┘                                       │
		//│       ax        │                                                         │
		//├────────┬────────┤                                                         │
		//│   al   │   ah   │                                                         │
		//└────────┴────────┴─────────────────────────────────────────────────────────┘

		auto full {rg.bank[rg.slot].first};

		if (bytes == 1)
		{
			if (full == u8"rax") return u8"al";
			if (full == u8"rbx") return u8"bl";
			if (full == u8"rcx") return u8"cl";
			if (full == u8"rdx") return u8"dl";
		}
		if (bytes == 2)
		{
			if (full == u8"rax") return u8"ax";
			if (full == u8"rbx") return u8"bx";
			if (full == u8"rcx") return u8"cx";
			if (full == u8"rdx") return u8"dx";
		}
		if (bytes == 4)
		{
			if (full == u8"rax") return u8"eax";
			if (full == u8"rbx") return u8"ebx";
			if (full == u8"rcx") return u8"ecx";
			if (full == u8"rdx") return u8"edx";
		}
		return full;
	}
	
	inline constexpr auto view_fpr(const reg_t& rx, size_t bytes) -> utf8	
	{
		//┌──────────┬──────────┬───────────────────────┐
		//│ 128 bits │ 128 bits │       256 bits        │
		//├──────────┴──────────┴───────────────────────┤
		//│                    zmm                      │
		//├─────────────────────┬───────────────────────┤
		//│         ymm         │                       │
		//├──────────┬──────────┘                       │
		//│   xmm    │                                  │
		//└──────────┴──────────────────────────────────┘

		auto full {rx.bank[rx.slot].first};

		if (bytes == 4)
		{
			if (full == u8"xmm0") return u8"xmm0";
			if (full == u8"xmm1") return u8"xmm1";
			if (full == u8"xmm2") return u8"xmm2";
			if (full == u8"xmm3") return u8"xmm3";
		}
		if (bytes == 8)
		{
			if (full == u8"xmm0") return u8"xmm0";
			if (full == u8"xmm1") return u8"xmm1";
			if (full == u8"xmm2") return u8"xmm2";
			if (full == u8"xmm3") return u8"xmm3";
		}
		return full;
	}

	//|-----------------|
	//| asm::arithmetic |
	//|-----------------|

	// add r1, r2 => r1 += r2
	inline /*Ი︵𐑼*/ auto cg_add(const ins_t* in, const reg_t& r1, const reg_t& r2) -> reg_t
	{
		//|-------------------------------------------------------|
		this->program += u8"\t%s %s, %s\n"_utf | in->add | r1 | r2;
		//|-------------------------------------------------------|

		r2.release(); return r1;
	}

	// sub r1, r2 => r1 -= r2
	inline /*Ი︵𐑼*/ auto cg_sub(const ins_t* in, const reg_t& r1, const reg_t& r2) -> reg_t
	{
		//|-------------------------------------------------------|
		this->program += u8"\t%s %s, %s\n"_utf | in->sub | r1 | r2;
		//|-------------------------------------------------------|

		r2.release(); return r1;
	}

	// mul r1, r2 => r1 *= r2
	inline /*Ი︵𐑼*/ auto cg_mul(const ins_t* in, const reg_t& r1, const reg_t& r2) -> reg_t
	{
		//|-------------------------------------------------------|
		this->program += u8"\t%s %s, %s\n"_utf | in->mul | r1 | r2;
		//|-------------------------------------------------------|

		r2.release(); return r1;
	}

	inline /*Ი︵𐑼*/ auto cg_div(const ins_t* in, const reg_t& r1, const reg_t& r2) -> reg_t
	{
		assert(!"<ERROR>");
		std::unreachable();
		return {nullptr, 0};
	}

	inline /*Ი︵𐑼*/ auto cg_mod(const ins_t* in, const reg_t& r1, const reg_t& r2) -> reg_t
	{
		assert(!"<ERROR>");
		std::unreachable();
		return {nullptr, 0};
	}

	inline /*Ი︵𐑼*/ auto cg_pow(const ins_t* in, const reg_t& r1, const reg_t& r2) -> reg_t
	{
		assert(!"<ERROR>");
		std::unreachable();
		return {nullptr, 0};
	}

	inline /*Ი︵𐑼*/ auto cg_load(int raw) -> reg_t
	{
		auto r1 {this->pull_gpr()};

		this->program += u8"\tmov %s, %s\n"_utf | r1 | raw;

		return r1; // allocation..!
	}	

	inline /*Ი︵𐑼*/ auto cg_load(long raw) -> reg_t
	{
		auto r1 {this->pull_gpr()};

		this->program += u8"\tmov %s, %s\n"_utf | r1 | raw;

		return r1; // allocation..!
	}

	inline /*Ი︵𐑼*/ auto cg_load(float raw) -> reg_t
	{
		auto rg {this->pull_gpr()};
		auto rx {this->pull_fpr()};

		uint32_t bits {0};
		std::memcpy(&bits, &raw, sizeof(raw));
		static_assert(sizeof(bits) == sizeof(raw));

		this->program += u8"\tmov %s, %s\n"_utf | this->view_gpr(rg, 4) | bits;
		this->program += u8"\tmovd %s, %s\n"_utf | rx | this->view_gpr(rg, 4);

		return rx; // allocation..!
	}

	inline /*Ი︵𐑼*/ auto cg_load(double raw) -> reg_t
	{
		auto rg {this->pull_gpr()};
		auto rx {this->pull_fpr()};

		uint64_t bits {0};
		std::memcpy(&bits, &raw, sizeof(raw));
		static_assert(sizeof(bits) == sizeof(raw));

		this->program += u8"\tmov %s, %s\n"_utf | this->view_gpr(rg, 8) | bits;
		this->program += u8"\tmovq %s, %s\n"_utf | rx | this->view_gpr(rg, 8);

		return rx; // allocation..!
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
