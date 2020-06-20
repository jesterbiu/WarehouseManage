#include "order_test.hpp"
#include "manage_test.hpp"
#define TRUE 1
#define FALSE 0

using namespace Tests;

// Test options
#define TEST_ITEM TRUE
#define TEST_ORDER FALSE

int main()
{
#if TEST_ITEM
	item_manager_test();
#endif

#if TEST_ORDER
	order_test();
#endif
}
