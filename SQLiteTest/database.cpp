#include "database.hpp"

using namespace warehouse;

// The 2nd argument to the sqlite3_exec() callback function is the number of columns in the result.
// The 3rd argument to the sqlite3_exec() callback is an array of pointers to strings obtained 
//		as if from sqlite3_column_text(), one for each column.
//		If an element of a result row is NULL then the corresponding string pointer for the sqlite3_exec() callback is a NULL pointer.
//The 4th argument to the sqlite3_exec() callback is an array of pointers to strings 
//		where each entry represents the name of corresponding result column 
//		as obtained from sqlite3_column_name().
int create_table_callback(void* outside, int row_count, char** col_vals, char** col_names)
{
	std::cout << "create table"
		<< "\nrow count: " << row_count << std::endl;
	if ((col_names) && (*col_names))
	{
		std::cout << col_names[0] << "\n";
	}
	else
	{
		std::cout << "ERROR: failed to extract column name(s)\n";
	}
	if ((col_vals) && (*col_vals))
	{
		std::cout << col_vals[0] << "\n";
	}
	else
	{
		std::cout << "ERROR: failed to extract column value(s)\n";
	}
	return 0;
}

database::database()
{
	// Open database instance
	auto rc = sqlite3_open(":memory:", &db);
	if (SQLITE_OK != rc)
	{
		std::cout << sqlite3_errmsg(db);
		db = nullptr;
		throw warehouse_exception("database::database(): sqlite3_open() failed!", rc);
	}

	create_item_table();
	create_order_table();
	create_refund_order_table();
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
		"CREATE TABLE items (shelf INTEGER, slot INTEGER, id TEXT PRIMARY KEY, stocks INTEGER)";
	create_table(sqlstmt);
}

void database::create_order_table()
{
	static const char sqlstmt_orders[] =
		"CREATE TABLE orders (order_id TEXT PRIMARY KEY, status INTEGER)";
	static const char sqlstmt_order_details[] =
		"CREATE TABLE order_details (order_id TEXT, item_id TEXT, quantity INTEGER)";
	create_table(sqlstmt_orders);
	create_table(sqlstmt_order_details);
}

void database::create_refund_order_table()
{
	static const char sqlstmt_refund_orders[] =
		"CREATE TABLE refund_orders (order_id TEXT, item_id TEXT, refund_quantity INTEGER)";
	create_table(sqlstmt_refund_orders);
}