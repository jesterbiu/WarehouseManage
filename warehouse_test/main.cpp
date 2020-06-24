#include "order_test.hpp"
#include "manage_test.hpp"
#include "pers_test.hpp"
#define TRUE 1
#define FALSE 0
using namespace Tests;

// Test options
#define TEST_ITEM 0
#define TEST_ORDER 0
#define TEST_PERSONNEL 1

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
}
