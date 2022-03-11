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

ctsm::state_t test_state_1(bool b);
ctsm::state_t test_state_2(bool &b);

ctsm::state_t test_state_1(bool b)
{
	return b ? ctsm::state<test_state_2> : ctsm::state<test_state_1>;
}
ctsm::state_t test_state_2(bool &b)
{
	b = !b;
	return !b ? ctsm::state<test_state_1> : ctsm::state<test_state_2>;
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

	auto behavior = test_behavior{ctsm::state<test_state_2>};
	EXPECT_EQ(behavior.state(), ctsm::state<test_state_2>);

	bool switch_state = false;
	EXPECT_EQ(behavior(switch_state), ctsm::state<test_state_2>);
	EXPECT_TRUE(switch_state);
	EXPECT_EQ(behavior(switch_state), ctsm::state<test_state_1>);
	EXPECT_FALSE(switch_state);
	EXPECT_EQ(behavior(switch_state), ctsm::state<test_state_1>);
}