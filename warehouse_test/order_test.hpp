#pragma once
#include <iostream>
#include <map>
#include "..\SQLiteTest\order_manager.hpp"
#include "..\SQLiteTest\storage.hpp"
namespace Tests
{
	void order_test();
	void refund_order_test(std::map<std::string, warehouse::Order>&, warehouse::order_manager&);
	bool operator ==(const std::vector<warehouse::good>& lhs, const std::vector<warehouse::good>& rhs);
}