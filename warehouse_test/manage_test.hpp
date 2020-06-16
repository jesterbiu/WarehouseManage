#pragma once

#include "..\SQLiteTest\storage.hpp"
#include "..\SQLiteTest\item_manager.hpp"
#include <iostream>

namespace Tests
{
	void manage_test();
	void test_part_two(warehouse::item_manager&);
}