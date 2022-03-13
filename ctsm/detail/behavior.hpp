//
// Created by switchblade on 2022-03-11.
//

#pragma once

#include <exception>

#include "state.hpp"

namespace ctsm::detail
{
	/** @brief Template behavior type.
	 * @tparam States Functions invoked as states of the behavior.
	 * @note State functions must all be invocable from the same arguments & return a valid `state_t` value
	 * (`state_t` value is considered valid if it is identifying one of the `States` state functions). */
	template<auto... States> requires state_pack<States...>
	class behavior
	{
		template<size_t I, size_t J, auto S, auto... Rest>
		constexpr static auto get_state() noexcept
		{
			if constexpr(I == J)
				return S;
			else
				return get_state<I, J + 1, Rest...>();
		}

	public:
		/** Initializes the behavior with the default (first parameter of the pack) state. */
		constexpr behavior() noexcept : next_state(default_state<States...>::value) {}
		/** Initializes the behavior with the specified state. */
		constexpr explicit behavior(state_t state) noexcept : next_state(state) {}

		/** Invokes the current state of the behavior using the passed arguments.
		 * @param args Arguments passed to the state.
		 * @return The next state to be executed, or `bad_state` if an unrecognized state was returned
		 * or the current state cannot be invoked with `args`.
		 * @note All states must be invocable with the passed arguments. */
		template<typename... Args>
		constexpr state_t operator()(Args &&...args)
		{
			return invoke_state(std::forward<Args>(args)...);
		}
		/** Returns `state_t` value of the next state function to be executed. */
		[[nodiscard]] constexpr state_t state() const noexcept { return next_state; }

	private:
		template<size_t I = 0, auto S = get_state<I, 0, States...>(), typename... Args>
		constexpr state_t invoke_state(Args &&...args)
		{
			/* Only consider the state if it is invocable with the passed arguments. */
			if constexpr(requires{ S(std::forward<Args>(args)...); })
			{
				/* Unfortunately, a switch cannot be used since `state_t` uses a pointer.
				 * Runtime index generation cannot solve this since switch cases require constant expressions. */
				if (next_state == detail::state<S>) [[likely]]
					return next_state = S(std::forward<Args>(args)...);
			}

			if constexpr(I + 1 < sizeof...(States))
				return invoke_state<I + 1>(std::forward<Args>(args)...);
			else
				return bad_state;
		}

		/** State to be executed on the next call to `operator()` */
		state_t next_state;
	};
}