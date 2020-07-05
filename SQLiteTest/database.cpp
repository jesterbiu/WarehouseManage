#include "database.hpp"
// Use temperary database if 1 
#define MEMORY_DB 1

using namespace WarehouseManage;

database::database()
{
	// Open database instance
#if MEMORY_DB
	auto rc = sqlite3_open(":memory:", &db);
#endif
#if !MEMORY_DB
	auto rc = sqlite3_open(".\\data\\warehouse.db", &db);
#endif
	if (SQLITE_OK != rc)
	{
		std::cout << sqlite3_errmsg(db);
		db = nullptr;
		throw warehouse_exception("database::database(): sqlite3_open() failed!", rc);
	}

	create_item_table();
	create_order_table();
	create_refund_order_table();
	create_personnel_table();
}

void database::create_table(const char* sqlstmt)
{
	// Pointer to database error message
	char* errmsg = NULL;

	// Create the Item table
	auto rc = sqlite3_exec(db, sqlstmt, NULL, NULL, &errmsg);
	if (rc != SQLITE_OK)
	{
		if (errmsg)
		{
			std::cout << errmsg << "\n";
		}
		db = nullptr;
		throw warehouse_exception("database::create_table(): sqlite3_exec() failed!", rc);
	}
}

void database::create_item_table()
{
	static const char sqlstmt[] =
		"CREATE TABLE IF NOT EXISTS items (shelf INTEGER, slot INTEGER, id TEXT PRIMARY KEY, stocks INTEGER)";
	create_table(sqlstmt);
}

void database::create_order_table()
{
	static const char sqlstmt_orders[] =
		"CREATE TABLE IF NOT EXISTS orders (order_id TEXT PRIMARY KEY, status INTEGER)";
	static const char sqlstmt_order_details[] =
		"CREATE TABLE IF NOT EXISTS order_details (order_id TEXT, item_id TEXT, quantity INTEGER)";
	create_table(sqlstmt_orders);
	create_table(sqlstmt_order_details);
}

void database::create_refund_order_table()
{
	static const char sqlstmt_refund_orders[] =
		"CREATE TABLE IF NOT EXISTS refund_orders (order_id TEXT, item_id TEXT, refund_quantity INTEGER)";
	create_table(sqlstmt_refund_orders);
}

void database::create_personnel_table()
{
	static const char sqlstmt_personnels[] =
		"CREATE TABLE IF NOT EXISTS personnels (role INTEGER, pers_id TEXT PRIMARY KEY, password TEXT)";
	create_table(sqlstmt_personnels);
}
