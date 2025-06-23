#pragma once

#include <tuple>
#include <utility>
#include <cstddef>

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
			typename V
		>
		requires requires(V&& impl, const T& crtp)
		{
			std::forward<V>(impl)(static_cast<const T&>(crtp));
		}
		inline constexpr auto accept(V&& impl) const
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
	typename... Fs
>
struct visitor : Fs...
{
	using Fs::operator()...;

	inline constexpr void operator()(auto&) const
	{
		static_assert(false, "unsupported type");
	}
};

template
<
	typename... Fs
>
visitor(Fs...) -> visitor<Fs...>;

//|-----------|
//| fix-point |
//|-----------|

template
<
	typename Fs
>
struct anchor
{
	Fs f;

	template
	<
		typename... X
	>
	inline constexpr auto operator()(X&&... x)
	{
		return f(*this, std::forward<X>(x)...);
	}
};
template
<
	typename... Fs
>
anchor(Fs...) -> anchor<Fs...>;

//|-----------|
//| fix point |
//|-----------|

template
<
	typename Fs
>
struct combine
{
	Fs f;

	template
	<
		typename... X
	>
	inline constexpr auto operator()(X&&... x)
	{
		return f(*this, std::forward<X>(x)...);
	}
};
template
<
	typename... Fs
>
combine(Fs...) -> combine<Fs...>;

//|-----------------|
//| helper function |
//|-----------------|

template
<
	typename    V,
	typename... L
>
requires requires(V&& base)
{
	[]<typename... Fs>(visitor<Fs...>&){}(base);
}
inline constexpr auto overrides(V&& base, L&&... lambda)
{
	auto handles {std::make_tuple(std::forward<L>(lambda)...)};

	return [&](auto&&... args)
	{
		auto iterate // dispatcher
		{
			[&]<size_t N = 0>(auto& recurse)
			{
				if constexpr (N < sizeof...(L))
				{
					const auto& handle {std::get<N>(handles)};

					if constexpr (requires { handle(args...); })
					{
						// invoke the handler with current set of args
						handle(std::forward<decltype(args)>(args)...);
					}
					else
					{
						// recursively try the next handle in the tuple
						recurse.template operator()<N + 1>(recurse);
					}
				}
				else
				{
					// if no override matches, fallback to base
					base(std::forward<decltype(args)>(args)...);
				}
			}
		};
		return iterate.template operator()<0>(iterate);
	};
}

template
<
	typename    V,
	typename... L
>
requires requires(V&& base)
{
	[]<typename Fs>(anchor<Fs>&){}(base);
}
inline constexpr auto overrides(V&& base, L&&... lambda)
{
	auto handles {std::make_tuple(std::forward<L>(lambda)...)};

	return anchor
	{
		[&](auto& self, auto&&... args) -> void
		{
			auto iterate // dispatcher
			{
				[&]<size_t N = 0>(auto& recurse)
				{
					if constexpr (N < sizeof...(L))
					{
						const auto& handle {std::get<N>(handles)};

						if constexpr (requires { handle(self, args...); })
						{
							// invoke the handler with current set of args
							handle(self, std::forward<decltype(args)>(args)...);
						}
						else
						{
							// recursively try the next handle in the tuple
							recurse.template operator()<N + 1>(recurse);
						}
					}
					else
					{
						// if no override matches, fallback to base
						base.f(self, std::forward<decltype(args)>(args)...);
					}
				}
			};
			iterate.template operator()<0>(iterate);
		}
	};
}
