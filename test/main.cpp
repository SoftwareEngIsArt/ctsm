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

namespace
{
	struct state_data
	{
		bool next = false;
		int ctr = 0;
	};

	constexpr ctsm::state_t test_state_1(state_data &);
	constexpr ctsm::state_t test_state_2(state_data &);

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
}

template
class ctsm::state_t::generator<test_state_1>;

TEST(ctsm_tests, state_test)
{
	static_assert(!std::is_default_constructible_v<ctsm::state_t>);

	EXPECT_NE(ctsm::state<test_state_1>, ctsm::state<test_state_2>);
	EXPECT_EQ(ctsm::state<test_state_1>, ctsm::state<test_state_1>);
	EXPECT_NE(ctsm::state<test_state_1>, ctsm::bad_state);
	EXPECT_EQ(ctsm::state<test_state_2>, ctsm::state<test_state_2>);
	EXPECT_NE(ctsm::state<test_state_2>, ctsm::bad_state);
	EXPECT_EQ(ctsm::bad_state, ctsm::bad_state);
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

#include <vector>
#include <algorithm>
#include <cstdlib>

struct vacuum
{
	static ctsm::state_t final_state()
	{
		/* Final state should not be executed itself. */
		return ctsm::bad_state;
	}
	static ctsm::state_t initial_state(vacuum &data)
	{
		bool is_clean = !data.rooms[data.current_room];

		printf("\t[INITIAL] Current room (%lu): %s\n", data.current_room, is_clean ? "clean" : "dirty");
		if (!is_clean)
			return ctsm::state<clean_state>;
		else
			return ctsm::state<find_dirty_neighbor_state>;
	}
	static ctsm::state_t clean_state(vacuum &data)
	{
		printf("\t[CLEAN] Cleaning %lu...\n", data.current_room);
		data.rooms[data.current_room] = false;
		if (data.min_clean > data.current_room)
			data.min_clean = data.current_room;
		return ctsm::state<find_dirty_neighbor_state>;
	}
	static ctsm::state_t find_dirty_neighbor_state(vacuum &data)
	{
		/* Inspect neighbour rooms & clean them.
		 * If the current room is <= minimum clean room number, and the current room is not 0,
		 * search rooms on the left, otherwise, search rooms on the right. */
		int direction;
		if (data.current_room && data.current_room <= data.min_clean)
			direction = -1;
		else
			direction = 1;

		for (;;)
		{
			if (data.current_room == data.rooms.size())
			{
				printf("\t[SEARCH] All clean\n");
				return ctsm::state<final_state>;
			} else if (data.rooms[data.current_room])
			{
				printf("\t[SEARCH] Next dirty: %lu\n", data.current_room);
				return ctsm::state<clean_state>;
			} else if (!data.current_room)
				direction = 1;
			data.current_room += direction;
		}
	}

	void generate_rooms(std::size_t n)
	{
		/* Generate 2xn grid of rooms with random dirtiness. */
		auto total_rooms = n * 2;
		rooms.reserve(total_rooms);
		while (total_rooms-- != 0)
			rooms.emplace_back(!!(rand() % 2));
	}
	void print_rooms() const
	{
		for (std::size_t i = 0; i < rooms.size(); i += 2)
		{
			puts("\t+-----+-----+");
			printf("\t| %03lu | %03lu |\n", i, i + 1);
			printf("\t|  %c  |  %c  |\n", rooms[i] ? 'x' : ' ', rooms[i + 1] ? 'x' : ' ');
		}
		puts("\t+-----+-----+");
	}

	void reset()
	{
		rooms.clear();
		current_room = 0;
		min_clean = -1;
	}

	std::vector<bool> rooms;
	std::size_t current_room;
	std::size_t min_clean;
};

TEST(ctsm_tests, vacuum_cleaner)
{
	ctsm::behavior<vacuum::initial_state,
	               vacuum::clean_state,
	               vacuum::find_dirty_neighbor_state,
	               vacuum::final_state> behavior;
	vacuum data;

	auto run_test = [&](int i)
	{
		printf("Test %d:\n", i);

		auto N = ((rand()) % 64) + 1;
		data.reset();
		data.generate_rooms(N); /* Generate 2xN random rooms. */
		data.print_rooms();
		data.current_room = rand() % N; /* Start in one of the first 2 rooms. */

		behavior.reset();
		while (behavior.state() != ctsm::state<vacuum::final_state>)
			behavior(data);

		EXPECT_TRUE(std::none_of(data.rooms.begin(), data.rooms.end(), [](auto b) { return b; }));
	};

	for (auto i = 0; i < 64; ++i)
		run_test(i);
}