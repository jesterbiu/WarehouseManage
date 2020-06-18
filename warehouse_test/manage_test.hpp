#pragma once

#include "..\SQLiteTest\storage.hpp"
#include "..\SQLiteTest\item_manager.hpp"
#include <iostream>

namespace Tests
{
	void item_manager_test();
	void item_manager_test_pt2(warehouse::item_manager&);
}