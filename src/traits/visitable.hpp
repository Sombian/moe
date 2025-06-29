#pragma once

#include <tuple>
#include <cstddef>
#include <utility>

namespace trait
{
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
		requires requires(V&& impl, T& crtp)
		{
			std::forward<V>(impl)(static_cast<T&>(crtp));
		}
		inline constexpr auto accept(V&& impl)
		{
			std::forward<V>(impl)(static_cast<T&>(*this));
		}

		template
		<
			class V
		>
		requires requires(V&& impl, const T& crtp)
		{
			std::forward<V>(impl)(static_cast<const T&>(crtp));
		}
		inline constexpr auto accept(const V&& impl) const
		{
			std::forward<V>(impl)(static_cast<const T&>(*this));
		}
	};
}

//|----------|
//| overload |
//|----------|

template
<
	class... L
>
struct visitor : L...
{
	using L::operator()...;

	inline constexpr auto operator()(auto&& _) const
	{
		static_assert(false, "missing handle");
	}
};

template
<
	class... L
>
visitor(L...) -> visitor<L...>;

//|-----------|
//| fix-point |
//|-----------|

template
<
	class L
>
struct recurse
{
	L f;

	template
	<
		class... X
	>
	inline constexpr auto operator()(X&&... _) const
	{
		return f(*this, std::forward<X>(_)...);
	}
};

template
<
	class... L
>
recurse(L...) -> recurse<L...>;

//|-----------------|
//| helper function |
//|-----------------|

template
<
	class    V,
	class... L
>
requires requires(V&& target)
{
	[]<class... L2>(visitor<L2...>&){}(target);
}
inline constexpr auto overrides(V&& target, L&&... lambda)
{
	auto handles {std::make_tuple(std::forward<L>(lambda)...)};

	return [target, handles = std::move(handles)](auto&&... args) -> void
	{
		auto match // dispatcher
		{
			[&]<size_t N = 0>(auto& impl)
			{
				if constexpr (N < sizeof...(L))
				{
					//|-------------<nth lambda>-------------|
					const auto& handle {std::get<N>(handles)};
					//|--------------------------------------|

					if constexpr (requires { handle(args...); })
					{
						// invoke the handler with current set of args
						return handle(std::forward<decltype(args)>(args)...);
					}
					else
					{
						return impl.template operator()<N + 1>(impl);
					}
				}
				else
				{
					if constexpr (requires { target(args...); })
					{
						// if no override matches, fallback to base
						return target(std::forward<decltype(args)>(args)...);
					}
					else
					{
						static_assert(false, "missing base handle");
					}
				}
			}
		};
		return match.template operator()<0>(match);
	};
}

template
<
	class    V,
	class... L
>
requires requires(V&& target)
{
	[]<class L2>(recurse<L2>&){}(target);
}
inline constexpr auto overrides(V&& target, L&&... lambda)
{
	auto handles {std::make_tuple(std::forward<L>(lambda)...)};

	return recurse
	{
		[target, handles = std::move(handles)](auto& self, auto&&... args) -> void
		{
			auto match // dispatcher
			{
				[&]<size_t N = 0>(auto& impl)
				{
					if constexpr (N < sizeof...(L))
					{
						//|-------------<nth lambda>-------------|
						const auto& handle {std::get<N>(handles)};
						//|--------------------------------------|

						if constexpr (requires { handle(self, args...); })
						{
							// invoke the handler with current set of args
							return handle(self, std::forward<decltype(args)>(args)...);
						}
						else
						{
							return impl.template operator()<N + 1>(impl);
						}
					}
					else
					{
						if constexpr (requires { target.f(self, args...); })
						{
							// if no override matches, fallback to base
							return target.f(self, std::forward<decltype(args)>(args)...);
						}
						else
						{
							static_assert(false, "missing proper handle");
						}
					}
				}
			};
			return match.template operator()<0>(match);
		}
	};
}
