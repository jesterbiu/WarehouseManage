#include "item_test.hpp"
#include "order_test.hpp"
#include "pers_test.hpp"
#include "warehouse_test.hpp"
#define TRUE 1
#define FALSE 0
using namespace Tests;

// Test options
// WAREHOUSE conflicts with the other 3
#define TEST_ITEM 0
#define TEST_ORDER 0
#define TEST_PERSONNEL 0
#define TEST_WAREHOUSE 1

int main()
{
#if TEST_ITEM
	item_manager_test();
#endif

#if TEST_ORDER
	order_test();
#endif

#if TEST_PERSONNEL
	pers_test();
#endif

#if TEST_WAREHOUSE
	test_warehouse();
#endif
}
