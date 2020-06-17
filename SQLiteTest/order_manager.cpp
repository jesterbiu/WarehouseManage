#include "order_manager.hpp"

using namespace warehouse;

bool order_manager::add_order(const Order& order)
{
	// Validate input item
	if (!order) return false;

	// Execute insertion
	return insert_order(order) && insert_detail(order)
		? true
		: false;
}

bool order_manager::insert_order(const Order& order)
{
	// Prepare statement
	auto order_stmth = order_statement_generator::insert_order_stmt(get_database());
	if (!order_stmth)
	{
		return false;
	}

	// Bind SQL params
	bind_insert_order(order, *order_stmth);

	// Execute order insertion 
	return SQLITE_DONE == step(*order_stmth)
		? true
		: false;
}

bool order_manager::insert_detail(const Order& order)
{
	for (auto& g : order.goods)
	{
		// Prepare statement
		auto detail_stmth 
			= order_statement_generator::insert_order_detail_stmt(get_database());
		if (!detail_stmth)
		{
			return false;
		}

		// Bind SQL params
		bind_insert_detail(order.order_id, g, *detail_stmth);

		// Execute detail insertion
		if (SQLITE_DONE != step(*detail_stmth))
		{
			return false;
		}
	}

	return true;
}

bool order_manager::exist(const std::string& order_id)
{
	// Prepare statement
	auto stmthandle = order_statement_generator::order_exist_stmt(get_database());
	if (!stmthandle)
	{
		return false;
	}

	// Bind SQL param
	sqlite3_bind_text(*stmthandle, 1, order_id.c_str(), order_id.size(), nullptr);

	// Execute query
	auto rc = step(*stmthandle);
	if (rc != SQLITE_ROW)
	{
		throw warehouse_except("order_manager::exist(): step() failed to generate result!");
	}
	
	// Extract result
	auto count = sqlite3_column_int(*stmthandle, 0);
	switch (count)
	{
	case 0:		// No such order
		return false;
	case 1:		// The order exists
		return true;
	default:	// count < 0: database error; 
				// count > 1: duplicate order
		throw warehouse_except("order_manager::exist(): impossible count of result!", count);
		break;
	}
	return false;
}

std::pair<bool, Order> order_manager::get_order(const std::string& order_id)
{
	// Check order existence
	if (!exist(order_id))
	{
		return std::make_pair(false, Order{});
	}

	// Prepare statement
	auto order_stmth 
		= order_statement_generator::query_order_stmt(get_database());
	if (!order_stmth)
	{
		return std::make_pair(false, Order{});
	}

	// Bind SQL param
	auto rc = 
		sqlite3_bind_text(*order_stmth, 1, order_id.c_str(), order_id.size(), nullptr);
	if (SQLITE_OK != rc)
	{
		throw warehouse_except("order_manager::get_order(): sqlite3_bind_text() failed!", rc);
	}

	// Extract result
	auto order = Order{};
	extract_order_query(order, *order_stmth);

	//-------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------

	// Detail
	auto detail_stmth =
		order_statement_generator::query_order_detail_stmt(get_database());

	// Bind SQL param
	rc = 
		sqlite3_bind_text(*detail_stmth, 1, order_id.c_str(), order_id.size(), nullptr);
	if (SQLITE_OK != rc)
	{
		throw warehouse_except("order_manager::get_order(): sqlite3_bind_text() failed!", rc);
	}

	// Extract results
	while ((rc = step(*detail_stmth)) == SQLITE_ROW)
	{
		extract_detail_query(order, *detail_stmth);
	}
	if (order.goods.empty())
	{
		throw warehouse_except("order_manager::get_order(): the order contains NO item!");
	}

	return rc == SQLITE_DONE
		? std::make_pair(true, order)
		: std::make_pair(false, order);
}

void order_manager::extract_order_query(Order& order, sqlite3_stmt* stmthandle)
{
	// order_id
	auto order_id = (const char*)sqlite3_column_text(stmthandle, 0);
	order.order_id = order_id;

	// Status
	auto int_status = sqlite3_column_int(stmthandle, 1);
	order.status = int_to_status(int_status);
}

void order_manager::extract_detail_query(Order& order, sqlite3_stmt* stmthandle)
{
	// item_id
	auto item_id_cstr = (const char*)sqlite3_column_text(stmthandle, 0);
	auto item_id = std::string{ item_id_cstr };

	// quantity
	auto quantity = sqlite3_column_int(stmthandle, 1);

	// Add to goods
	order.goods.emplace_back(item_id, quantity);
}

void order_manager::bind_insert_order(const Order& order, sqlite3_stmt* order_stmth)
{
	// Bind order_id
	auto order_id = order.order_id.c_str();
	auto rc1
		= sqlite3_bind_text(order_stmth, 1, order_id, order.order_id.size(), nullptr);

	// Bind status
	auto status = status_to_int(order.status);
	auto rc2 = sqlite3_bind_int(order_stmth, 2, status);

	// Validate result
	if (rc1 != SQLITE_OK
		|| rc2 != SQLITE_OK)
	{
		throw warehouse_except("order_manager::bind_insert_order(): bind failed!");
	}
}

void order_manager::bind_insert_detail(const std::string& order_id, const good& g, sqlite3_stmt* detail_stmth)
{
	// Bind order_id
	auto rc1
		= sqlite3_bind_text(detail_stmth, 1, order_id.c_str(), order_id.size(), nullptr);

	// Bind item_id
	auto& item_id = g.first;
	auto rc2
		= sqlite3_bind_text(detail_stmth, 2, item_id.c_str(), item_id.size(), nullptr);

	// Bind quantity
	auto& quantity = g.second;
	auto rc3
		= sqlite3_bind_int(detail_stmth, 3, quantity);

	// Validate result
	if (rc1 != SQLITE_OK
		|| rc2 != SQLITE_OK
		|| rc3 != SQLITE_OK)
	{
		throw warehouse_except("order_manager::bind_insert_order(): bind failed!");
	}
}