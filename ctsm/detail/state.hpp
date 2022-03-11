//
// Created by switchblade on 2022-03-11.
//

#pragma once

#include <type_traits>
#include <cstdint>

namespace ctsm::detail
{
	class state_t;

	template<auto>
	struct state_func_impl : std::false_type {};
	template<typename... Args, state_t (*S)(Args...)>
	struct state_func_impl<S> : std::true_type {};
	template<auto State>
	concept state_func = state_func_impl<State>::value;

	/** @brief Type used to uniquely identify a state. */
	class state_t
	{
		using dummy_t = uint8_t; /* Type does not matter, uint8_t used to avoid wasting memory. */

		constexpr explicit state_t(const dummy_t &dummy) noexcept : id_dummy(&dummy) {}

	public:
		template<auto State> requires state_func<State>
		class generator
		{
			constinit static const dummy_t dummy;

		public:
			/** Returns an instance of `state_t` for the specific state. */
			[[nodiscard]] constexpr state_t operator()() const noexcept { return state_t{dummy}; }
		};

	public:
		state_t() = delete;

		[[nodiscard]] constexpr bool operator==(const state_t &) const noexcept = default;
		[[nodiscard]] constexpr bool operator!=(const state_t &) const noexcept = default;

	private:
		const dummy_t *id_dummy;
	};

	template<auto S> requires state_func<S>
	constinit const uint8_t state_t::generator<S>::dummy = {};

	/** @brief Variable used to generate state identifier. */
	template<auto S> requires state_func<S>
	constexpr state_t state = state_t::generator<S>{}();
	template<auto S, auto...>
	struct default_state { constexpr static auto value = state<S>; };

	template<auto... States>
	concept state_pack = sizeof...(States) != 0 && std::conjunction_v<state_func_impl<States>...>;
}