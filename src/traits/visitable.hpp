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
		requires requires
		(
			V impl, T crtp
		)
		{
			impl.visit(crtp);
		}
		void accept(V&& impl)
		{
			// perfect forwarding + safe downcasting
			std::forward<V>(impl).visit(static_cast<T&>(*this));
		}
	};
}
