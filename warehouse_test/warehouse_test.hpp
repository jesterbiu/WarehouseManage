#include "..\SQLiteTest\warehouse.hpp"
#include <sstream>
#include <algorithm>
namespace Tests
{
	using namespace WarehouseManage;
	void add_items(warehouse& w);
	void add_orders(warehouse& w);
	void add_personnels(warehouse& w);

	void test_warehouse();
	void test_instock(warehouse& w);
	void test_pick(warehouse& w);
	void test_inv(warehouse& w);
	void test_refund(warehouse& w);
}