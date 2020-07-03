#include "..\SQLiteTest\warehouse.hpp"
#include <sstream>
#include <algorithm>
namespace Tests
{
	using namespace WarehouseManage;
	std::unique_ptr<item_manager> get_item_manager();
	std::unique_ptr<order_manager> get_order_manager();
	std::unique_ptr<personnel_manager> get_personnel_manager();
	void test_warehouse();
	void test_instock(warehouse& w);
	void test_pick(warehouse& w);
	void test_inv(warehouse& w);
	void test_refund(warehouse& w);
}