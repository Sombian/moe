#pragma once

#include <utility>

template
<
	typename T
>
struct visitable
{
	template
	<
		typename V
	>
	requires requires(V&& impl, T&& crtp)
	{
		std::forward<V>(impl)(crtp);
	}
	inline constexpr auto accept(V&& impl)
	{
		std::forward<V>(impl)(*this);
	}
};

template
<
	typename L
>
// anchor
struct fix
{
	L f;

	template
	<
		typename... X
	>
	inline constexpr auto operator()(X&&... x) const -> decltype(auto)
	{
		return f(*this, std::forward<X>(x)...);
	}
};

template
<
	typename... L
>
fix(L...) -> fix<L...>;

template
<
	typename... L
>
// visitor
struct visit : L...
{
	using L::operator()...;

	template
	<
		typename... X
	>
	inline constexpr auto operator()(X&&... x) const -> decltype(auto)
	{
		// static_assert(sizeof...(X) == 0);
	}
};

template
<
	typename... L
>
visit(L...) -> visit<L...>;

template
<
	typename    T,
	typename... L
>
inline constexpr auto visitor(L&&... l)
{
	return [impl = visit<L...>{std::forward<L>(l)...}](auto&&... arg) -> T
	{
		return impl(std::forward<decltype(arg)>(arg)...); // lambda wrapper
	};
}
