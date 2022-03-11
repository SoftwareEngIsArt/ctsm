//
// Created by switchblade on 2022-03-11.
//

#pragma once

#include <type_traits>
#include <cstdint>

namespace ctsm::detail
{
	template<auto>
	struct state_func_impl : std::false_type {};

	/** @brief Type used to uniquely identify a state. */
	class state_t
	{
		template<auto>
		friend
		struct state_func_impl;

		constexpr explicit state_t(const void **id) noexcept : id(id) {}

	public:
		state_t() = delete;

		[[nodiscard]] constexpr bool operator==(const state_t &) const noexcept = default;
		[[nodiscard]] constexpr bool operator!=(const state_t &) const noexcept = default;

	private:
		const void **id;
	};

	template<typename R, typename... Args, R (*S)(Args...)>
	struct state_func_impl<S> : std::is_same<R, state_t>
	{
		/* Use a void pointer as id for the state.
		 * TODO: Make sure id is exported on windows to *hopefully* avoid multiple ids across DLL boundaries. */
		constinit static const void *id;
		constexpr static state_t instance = state_t{&id};
	};
	template<typename R, typename... Args, R (*S)(Args...)>
	constinit const void *state_func_impl<S>::id = nullptr;
	template<auto State>
	concept state_func = state_func_impl<State>::value;

	/** @brief Variable used to generate state identifier. */
	template<auto S> requires state_func<S>
	constexpr state_t state = state_func_impl<S>::instance;
	template<auto S, auto...>
	struct default_state { constexpr static auto value = state<S>; };

	template<auto... States>
	concept state_pack = sizeof...(States) != 0 && std::conjunction_v<state_func_impl<States>...>;
}