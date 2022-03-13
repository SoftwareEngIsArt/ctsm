//
// Created by switchblade on 2022-03-11.
//

#include <gtest/gtest.h>

#include "ctsm.hpp"

int main(int argc, char **argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

struct state_data
{
	bool next = false;
	int ctr = 0;
};

constexpr ctsm::state_t test_state_1(state_data &);
constexpr ctsm::state_t test_state_2(state_data &);

template
class ctsm::state_t::generator<test_state_1>;

constexpr ctsm::state_t test_state_1(state_data &d)
{
	d.ctr++;
	return d.next ? ctsm::state<test_state_2> : ctsm::state<test_state_1>;
}
constexpr ctsm::state_t test_state_2(state_data &d)
{
	d.next = !d.next;
	return !d.next ? ctsm::state<test_state_1> : ctsm::state<test_state_2>;
}

TEST(ctsm_tests, behavior_test)
{
	using test_behavior = ctsm::behavior<test_state_1, test_state_2>;

	static_assert(std::is_default_constructible_v<test_behavior>);
	static_assert(std::is_copy_constructible_v<test_behavior>);
	static_assert(std::is_copy_assignable_v<test_behavior>);
	static_assert(std::is_move_constructible_v<test_behavior>);
	static_assert(std::is_move_assignable_v<test_behavior>);
	static_assert(std::is_trivially_copyable_v<test_behavior>);
	static_assert(std::is_trivially_destructible_v<test_behavior>);

	auto behavior = test_behavior{ctsm::state<test_state_2>};
	EXPECT_EQ(behavior.state(), ctsm::state<test_state_2>);
	EXPECT_TRUE(behavior.valid());

	state_data data = {};
	EXPECT_EQ(behavior(data), ctsm::state<test_state_2>);
	EXPECT_TRUE(data.next);
	EXPECT_EQ(data.ctr, 0);
	EXPECT_EQ(behavior(data), ctsm::state<test_state_1>);
	EXPECT_FALSE(data.next);
	EXPECT_EQ(data.ctr, 0);
	EXPECT_EQ(behavior(data), ctsm::state<test_state_1>);
	EXPECT_EQ(data.ctr, 1);

	EXPECT_EQ(behavior(), ctsm::bad_state);
	EXPECT_FALSE(behavior.valid());

	behavior.reset();
	EXPECT_TRUE(behavior.valid());
	EXPECT_EQ(behavior.state(), ctsm::state<test_state_1>);
}