#pragma once

#include <utility>

template
<
	class T
>
struct visitable
{
	template
	<
		class V
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
	class L
>
// anchor
struct fix
{
	L f;

	template
	<
		class... X
	>
	inline constexpr auto operator()(X&&... x) const -> decltype(auto)
	{
		return f(*this, std::forward<X>(x)...);
	}
};

template
<
	class... L
>
fix(L...) -> fix<L...>;

template
<
	class... L
>
// visitor
struct visit : L...
{
	using L::operator()...;

	template
	<
		class... X
	>
	inline constexpr auto operator()(X&&... x) const -> decltype(auto)
	{
		// static_assert(sizeof...(X) == 0);
	}
};

template
<
	class... L
>
visit(L...) -> visit<L...>;

template
<
	class    T,
	class... L
>
inline constexpr auto visitor(L&&... l)
{
	return [impl = visit<L...>{std::forward<L>(l)...}](auto&&... arg) -> T
	{
		return impl(std::forward<decltype(arg)>(arg)...); // lambda wrapper
	};
}
