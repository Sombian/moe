#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <variant>

#include "core/fs.hpp"

#include "models/tst.hpp"

#include "utils/unicode.hpp"
#include "utils/ordering.hpp"

#include "lang/common/eof.hpp"
#include "lang/common/token.hpp"
#include "lang/common/error.hpp"
#include "lang/common/trail.hpp"

template
<
	type::string A,
	type::string B
>
class lexer
{
	//|-----<file>-----|
	fs::file<A, B>* src;
	//|----------------|
	trail jar;
	uint16_t x;
	uint16_t y;
	decltype(src->data.begin()) it;
	decltype(&src->data.begin()) ptr {0};
	decltype(*src->data.begin()) out {0};

	#define T(value) token<A, B> \
	{                            \
		this->x,                 \
		this->y,                 \
		*this,                   \
		value,                   \
		{                        \
			this->ptr,           \
			&this->it,           \
		},                       \
	}                            \

	#define E(value) error<A, B> \
	{                            \
		this->x,                 \
		this->y,                 \
		*this,                   \
		value,                   \
	}                            \

	auto next() -> char32_t
	{
		//|------------------|
		this->out = *this->it;
		//|------------------|
		++this->it;

		switch (this->out)
		{
			case '\n':
			{
				++this->jar.y();
				break;
			}
			default:
			{
				++this->jar.x();
				break;
			}
		}
		return this->out;
	}

	auto back() -> char32_t
	{
		--this->it;
		//|------------------|
		this->out = *this->it;
		//|------------------|

		switch (this->out)
		{
			case '\n':
			{
				--this->jar.y();
				break;
			}
			default:
			{
				--this->jar.x();
				break;
			}
		}
		return this->out;
	}

public:

	lexer
	(
		decltype(src) file
	)
	: src {file}, it {file->data.begin()} {}

	//|-----------------|
	//| member function |
	//|-----------------|

	operator fs::file<A, B>*()
	{
		return this->src;
	}

	inline constexpr auto pull() -> std::variant<token<A, B>, error<A, B>, eof>
	{
		for
		(
			this->ptr = &this->it, this->x = this->jar.x(), this->y = this->jar.y()
			;
			this->next()
			;
			this->ptr = &this->it, this->x = this->jar.x(), this->y = this->jar.y()
		)
		{
			switch (this->out)
			{
				case '/':
				{
					switch (*this->it)
					{
						case '/': { this->skip_1_line_comment(); continue; }
						case '*': { this->skip_N_line_comment(); continue; }
					}
					goto symbol;
				}
				case ' ':
				case '\t':
				case '\n':
				{
					continue;
				}
				case '\'':
				{
					code:
					return this->scan_1_code();
				}
				case '\"':
				{
					text:
					return this->scan_N_code();
				}
				case '0':
				{
					switch (*this->it)
					{
						case 'b':
						{
							bin:
							return this->scan_bin();
						}
						case 'o':
						{
							oct:
							return this->scan_oct();
						}
						case 'x':
						{
							hex:
							return this->scan_hex();
						}
					}
					goto number;
				}
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					number:
					return this->scan_num();
				}
				default:
				{
					symbol:
					return this->scan_sym();
				}
			}
		}
		return eof {};
	}

private:

	//|----------|
	//| comments |
	//|----------|

	inline constexpr auto skip_1_line_comment()
	{
		while (this->next())
		{
			if (this->out == '\n')
			{
				// this->next();
				break;
			}
		}
	}

	inline constexpr auto skip_N_line_comment()
	{
		while (this->next())
		{
			if (this->out == '*')
			{
				if (*this->it == '/')
				{
					this->next();
					break;
				}
			}
		}
	}

	//|----------------|
	//| string literal |
	//|----------------|

	inline constexpr auto scan_1_code() -> decltype(this->pull())
	{
		size_t len {0};

		for (; this->next() && this->out != '\''; ++len)
		{
			if (this->out == '\\') { this->next(); }
		}

		if (len != 1)
		{
			return E(u8"[lexer] code length must be 1");
		}
		if (!this->out)
		{
			return E(u8"[lexer] incomplete code literal");
		}
		return T(atom::CHAR);
	}

	inline constexpr auto scan_N_code() -> decltype(this->pull())
	{
		size_t len {0};
		
		for (; this->next() && this->out != '\"'; ++len)
		{
			if (this->out == '\\') { this->next(); }
		}

		if (!this->out)
		{
			return E(u8"[lexer] incomplete string literal");
		}
		return T(atom::TEXT);
	}

	//|----------------|
	//| number literal |
	//|----------------|

	inline constexpr auto scan_bin() -> decltype(this->pull())
	{
		// skip 'b'
		this->next();

		while (this->next())
		{
			switch (this->out)
			{
				// 0 ~ 1
				case '0':
				case '1':
				{
					break;
				}
				default:
				{
					this->back();
					goto bin_exit;
				}
			}
		}
		bin_exit:

		if (this->ptr - &this->it == 2)
		{
			return E(u8"[lexer] incomplete bin literal");
		}
		return T(atom::BIN);
	}

	inline constexpr auto scan_oct() -> decltype(this->pull())
	{
		// skip 'o'
		this->next();

		while (this->next())
		{
			switch (this->out)
			{
				// 0 ~ 7
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				{
					break;
				}
				default:
				{
					this->back();
					goto oct_exit;
				}
			}
		}
		oct_exit:

		if (this->ptr - &this->it == 2)
		{
			return E(u8"[lexer] incomplete oct literal");
		}
		return T(atom::OCT);
	}

	inline constexpr auto scan_hex() -> decltype(this->pull())
	{
		// skip 'x'
		this->next();

		while (this->next())
		{
			switch (this->out)
			{
				// 0 ~ 9
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				// A ~ F
				case 'A':
				case 'B':
				case 'C':
				case 'D':
				case 'E':
				case 'F':
				{
					break;
				}
				default:
				{
					this->back();
					goto hex_exit;
				}
			}
		}
		hex_exit:

		if (this->ptr - &this->it == 2)
		{
			return E(u8"[lexer] incomplete hex literal");
		}
		return T(atom::HEX);
	}

	inline constexpr auto scan_num() -> decltype(this->pull())
	{
		auto type {atom::INT};

		while (this->next())
		{
			switch (this->out)
			{
				// 0 ~ 9
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
				{
					break;
				}
				case '.':
				{
					if (type == atom::INT)
					{
						switch (*this->it)
						{
							// 0 ~ 9
							case '0':
							case '1':
							case '2':
							case '3':
							case '4':
							case '5':
							case '6':
							case '7':
							case '8':
							case '9':
							{
								type = atom::DEC;
								// (⸝⸝ᵕᴗᵕ⸝⸝)
								this->next();
								// (๑•᎑•๑)
								continue;
							}
						}
					}
					[[fallthrough]];
				}
				default:
				{
					this->back();
					goto num_exit;
				}
			}
		}
		num_exit:

		return T(type);
	}

	//|-----------|
	//| TST magic |
	//|-----------|

	inline constexpr auto scan_sym() -> decltype(this->pull())
	{
		#define macro(K, V) { V, atom::K },
		
		static const tst<atom> TBL
		{
			delimeters(macro)
			operator_l(macro)
			operator_i(macro)
			operator_r(macro)
			keywords(macro)
			special(macro)
		};
		#undef macro

		auto view {TBL.view()};

		// counter
		auto t_len {view[this->out] ? 1 : 0};
		auto props {utils::props(this->out)};
		auto s_len {props.XID_Start ? 1 : 0};

		// infinite loop fix
		if (t_len == 0 && s_len == 0)
		{
			return E(u8"[lexer] expects XID_Start");
		}

		// skip unnecessary iteration
		if (!(s_len != 0 && view.is_leaf()))
		{
			for (size_t i {1}; this->next(); ++i)
			{
				props = utils::props(this->out);
				
				// token check
				if (t_len == i && view[this->out])
				{
					++t_len;
				}
				// symbol check
				if (s_len == i && props.XID_Continue)
				{
					++s_len;
				}
				// if both fails
				if (t_len != i + 1 && s_len != i + 1)
				{
					this->back();
					break;
				}
			}
		}

		switch (utils::cmp(t_len, s_len))
		{
			case utils::ordering::LESS:
			{
				return T(atom::SYMBOL); // <- always symbol
			}
			case utils::ordering::EQUAL:
			{
				return T(view.get().value_or(atom::SYMBOL));
			}
			case utils::ordering::GREATER:
			{
				return T(view.get().value_or(atom::SYMBOL));
			}
		}
		assert(false && "-Wreturn-type");
	}

	#undef T
	#undef E
};
