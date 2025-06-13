#pragma once

#include <utility>

namespace traits
{
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
		requires requires(V impl, T crtp)
		{
			impl.visit(crtp);
		}
		inline constexpr void accept(V&& impl)
		{
			std::forward<V>(impl).visit(static_cast<T&>(*this));
		}

		template
		<
			typename V
		>
		requires requires(V impl, T crtp)
		{
			impl.visit(crtp);
		}
		inline constexpr void accept(const V&& impl) const
		{
			std::forward<V>(impl).visit(static_cast<const T&>(*this));
		}
	};
}
