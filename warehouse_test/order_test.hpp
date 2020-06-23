#pragma once
#include <iostream>
#include <map>
#include "..\SQLiteTest\order_manager.hpp"
#include "..\SQLiteTest\storage.hpp"
namespace Tests
{
	void order_test();
	void refund_order_test(std::map<std::string, WarehouseManage::Order>&, WarehouseManage::order_manager&);
	bool operator ==(const std::vector<WarehouseManage::good>& lhs, const std::vector<WarehouseManage::good>& rhs);
}