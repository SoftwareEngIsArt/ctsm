//
// Created by switchblade on 2022-03-11.
//

#pragma once

#include <type_traits>

namespace ctsm::detail
{
	class state_t;

	template<auto>
	struct state_func_impl : std::false_type {};
	template<typename... Args, state_t (*S)(Args...)>
	struct state_func_impl<S> : std::true_type {};
	template<auto State>
	concept state_func = state_func_impl<State>::value;

	struct bad_state_helper;

	/** @brief Type used to uniquely identify a state function. */
	class state_t
	{
		friend struct bad_state_helper;

		using dummy_t = char; /* Type does not matter, char used to avoid wasting memory. */

		constexpr explicit state_t(const dummy_t *dummy) noexcept : id_dummy(dummy) {}

	public:
		template<auto State> requires state_func<State>
		class generator
		{
			constinit static const dummy_t dummy;

		public:
			/** Returns an instance of `state_t` for the specific state. */
			[[nodiscard]] constexpr state_t operator()() const noexcept { return state_t{&dummy}; }
		};

	public:
		state_t() = delete;

		[[nodiscard]] constexpr bool operator==(const state_t &) const noexcept = default;
		[[nodiscard]] constexpr bool operator!=(const state_t &) const noexcept = default;

	private:
		const dummy_t *id_dummy;
	};

	template<auto S> requires state_func<S>
	constinit const state_t::dummy_t state_t::generator<S>::dummy = {};

	/** Variable used to generate state identifier. */
	template<auto S>
	constexpr state_t state = state_t::generator<S>{}();

	struct bad_state_helper { constexpr static auto value = state_t{nullptr}; };

	/** `state_t` value that is not identifying a state function. Used to indicate behavior state error. */
	constexpr state_t bad_state = bad_state_helper::value;

	template<auto S, auto...>
	struct default_state { constexpr static auto value = state<S>; };

	template<auto... States>
	concept state_pack = sizeof...(States) != 0 && std::conjunction_v<state_func_impl<States>...>;
}