//
// Created by switchblade on 2022-03-11.
//

#pragma once

#include <cstddef>
#include <utility>

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
		[[nodiscard]] constexpr static auto get_state() noexcept
		{
			if constexpr(I == J)
				return S;
			else
				return get_state<I, J + 1, Rest...>();
		}
		template<size_t I = 0, auto S = get_state<I, 0, States...>()>
		[[nodiscard]] constexpr static bool check_state(state_t state) noexcept
		{
			if (state == detail::state<S>)
				return true;
			else if constexpr(I + 1 < sizeof...(States))
				return check_state<I + 1>(state);
			else
				return false;
		}

	public:
		/** Initializes the behavior with the default (first parameter of the pack) state. */
		constexpr behavior() noexcept : next_state(default_state<States...>::value) {}
		/** Initializes the behavior with the specified state. */
		constexpr explicit behavior(state_t state) noexcept : next_state(state) {}

		/** Invokes the current state of the behavior using the passed arguments.
		 * @param args Arguments passed to the state.
		 * @return The next state to be executed, or `bad_state` if the behavior is in an unrecognized state
		 * or the state cannot be invoked with `args`.
		 * @note In case `bad_state` is returned, the behavior is guaranteed to be in an invalid state
		 * (it will have the `bad_state` value as it's state). */
		template<typename... Args>
		constexpr state_t operator()(Args &&...args)
		{
			return next_state = invoke_state(std::forward<Args>(args)...);
		}
		/** Resets the behavior to the specified state.
		 * @param new_state New state for the behavior. */
		constexpr void reset(state_t new_state = default_state<States...>::value) noexcept { next_state = new_state; }

		/** Returns `state_t` value of the next state function to be executed. */
		[[nodiscard]] constexpr state_t state() const noexcept { return next_state; }
		/** Checks if the behavior is in a valid (recognized) state. */
		[[nodiscard]] constexpr bool valid() const noexcept
		{
			return next_state != bad_state && check_state(next_state);
		}

	private:
		template<size_t I = 0, auto S = get_state<I, 0, States...>(), typename... Args>
		[[nodiscard]] constexpr state_t invoke_state(Args &&...args)
		{
			/* Only consider the state if it is invocable with the passed arguments. */
			if constexpr(requires{ S(std::forward<Args>(args)...); })
			{
				/* Unfortunately, a switch cannot be used since `state_t` uses a pointer.
				 * Runtime index generation cannot solve this since switch cases require constant expressions. */
				if (next_state == detail::state<S>) [[likely]]
					return S(std::forward<Args>(args)...);
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