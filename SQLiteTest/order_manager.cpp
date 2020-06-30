#include "order_manager.hpp"

using namespace WarehouseManage;

// Schemas:
// orders			(order_id TEXT, status	INTEGER)
// order_details	(order_id TEXT, item_id TEXT,	quantity		INTEGER)
// refund_orders	(order_id TEXT, item_id TEXT,	refund_quantity INTEGER)";

// add_order() and its helper functions
bool order_manager::add_order(const Order& order)
{
	if (exist(order.order_id)) { return false; }
	// Execute insertion
	return insert_order_part(order) && insert_detail_part(order)
		? true
		: false;
}
bool order_manager::insert_order_part(const Order& order)
{
	// Validate input
	if (!order)
	{
		return false;
	}

	// Prepare statement
	auto order_stmth 
		= order_statement_generator::insert_order_stmt(get_database());
	database::verify_stmt_handle(order_stmth);

	// Bind SQL params
	bind_insert_order_part(order, *order_stmth);

	// Execute order insertion 
	return SQLITE_DONE == database::step(*order_stmth)
		? true
		: false;
}
bool order_manager::insert_detail_part(const Order& order)
{
	// Validate input
	if (order.goods.empty())
	{
		return false;
	}

	for (auto& g : order.goods)
	{
		// Prepare statement
		auto detail_stmth 
			= order_statement_generator::insert_order_detail_stmt(get_database());
		database::verify_stmt_handle(detail_stmth);

		// Bind SQL params
		bind_insert_detail_part(order.order_id, g, *detail_stmth);

		// Execute detail insertion
		if (SQLITE_DONE != database::step(*detail_stmth))
		{
			return false;
		}
	}

	return true;
}
void order_manager::bind_insert_order_part(const Order& order, sqlite3_stmt* order_stmth)
{
	// Bind order_id
	auto rc1 = database::bind_text(order_stmth, 1, order.order_id);
	database::verify_binding(rc1);

	// Bind status
	auto status = status_to_int(order.status);
	auto rc2 = database::bind_int(order_stmth, 2, status);
	database::verify_binding(rc2);	
}
void order_manager::bind_insert_detail_part(const std::string& order_id, const good& g, sqlite3_stmt* detail_stmth)
{
	// Bind order_id
	auto rc1 = database::bind_text(detail_stmth, 1, order_id);
	database::verify_binding(rc1);

	// Bind item_id
	auto& item_id = g.first;
	auto rc2 = database::bind_text(detail_stmth, 2, item_id);
	database::verify_binding(rc2);

	// Bind quantity
	auto& quantity = g.second;
	auto rc3 = database::bind_int(detail_stmth, 3, quantity);
	database::verify_binding(rc3);
}

// exist()
bool order_manager::exist(const std::string& order_id)
{
	// Prepare statement
	auto stmthandle = order_statement_generator::order_exist_stmt(get_database());
	database::verify_stmt_handle(stmthandle);	

	// Bind SQL param
	auto rc = database::bind_text(*stmthandle, 1, order_id);
	database::verify_binding(rc);

	// Execute query
	rc = database::step(*stmthandle);
	if (!database::step_has_result(rc))
	{
		return true;
	}
	else
	{
		return false;
	}
	
	// Extract result
	auto count = database::extract_int(*stmthandle, 0);
	switch (count)
	{
	case 0:		// No such order
		return false;
	case 1:		// The order exists
		return true;
	default:	// count < 0: database error; 
				// count > 1: duplicate order
		throw warehouse_exception
		{ 
			"order_manager::exist(): impossible count of result!", count 
		};
		break;
	}
	return false;
}

// get_order() and its helper functions
std::pair<bool, Order> order_manager::get_order(const std::string& order_id)
{
	// Check order existence
	if (!exist(order_id))
	{
		return std::make_pair(false, Order{});
	}

	auto order = Order{ order_id };

	// Get record from database
	get_order_part(order);
	get_detail_part(order);

	return std::make_pair(true, order);
}
void order_manager::get_order_part(Order& order)
{
	// Prepare statement
	auto order_stmth
		= order_statement_generator::query_order_stmt(get_database());
	database::verify_stmt_handle(order_stmth);

	// Bind SQL param
	const auto& order_id = order.order_id;
	auto rc = database::bind_text(*order_stmth, 1, order_id);
	database::verify_binding(rc);

	// Extract result
	rc = database::step(*order_stmth);
	if (!database::step_has_result(rc))
	{
		throw warehouse_exception
		{
			"order_manager::get_order(): query return empty!",
			rc
		};
	}
	extract_order_query(order, *order_stmth);
}
void order_manager::get_detail_part(Order& order)
{
	// Prepare statement
	auto detail_stmth =
		order_statement_generator::query_order_detail_stmt(get_database());
	database::verify_stmt_handle(detail_stmth);	
	
	// Bind SQL param
	auto rc = database::bind_text(*detail_stmth, 1, order.order_id);
	database::verify_binding(rc);

	// Extract results
	while ((rc = database::step(*detail_stmth)) == SQLITE_ROW)
	{
		extract_detail_query(order, *detail_stmth);
	}
	if (order.goods.empty())
	{
		throw warehouse_exception
		{ 
			"order_manager::get_order(): the order contains NO item!" 
		};
	}
}
void order_manager::extract_order_query(Order& order, sqlite3_stmt* stmthandle)
{
	// order_id
	order.order_id = database::extract_text(stmthandle, 0);

	// Status
	auto int_status = database::extract_int(stmthandle, 1);
	order.status = int_to_status(int_status);
}
void order_manager::extract_detail_query(Order& order, sqlite3_stmt* stmthandle)
{
	// item_id
	auto item_id = database::extract_text(stmthandle, 0);

	// quantity
	auto quantity = database::extract_int(stmthandle, 1);

	// Add to goods
	order.goods.emplace_back(item_id, quantity);
}

// add_refund_order() and its helper functions
bool order_manager::add_refund_order(const Refund_Order& ro)
{
	// Validate order
	// Check order existence, return false if the order doesn't exist
	if (!exist(ro.order_id))
	{
		return false;
	}
	// Check order status, return false if status isn't Refunded
	else
	{
		auto c = check_status(ro.order_id);
		if (!c.first
			|| c.second != Order_Status_Type::Refunded)
		{
			return false;
		}
	}

	// TO BE ADDED: Validate refunds
	// refunded <= ordered
	// use get_refund_order()

	// Execute refunds insertion
	if (ro.refund_goods.empty())
	{
		return false;
	}
	for (const auto& rg : ro.refund_goods)
	{
		// Prepare SQL
		auto stmthandle
			= order_statement_generator::insert_refund_order_stmt(get_database());
		database::verify_stmt_handle(stmthandle);

		// Bind SQL params
		bind_insert_refund(ro.order_id, rg, *stmthandle);
		
		// step
		if (SQLITE_DONE != database::step(*stmthandle))
		{
			return false;
		}
	}
	return true;
}
void order_manager::bind_insert_refund(const std::string & order_id, const good& g, sqlite3_stmt* stmthandle)
{
	// Bind order_id
	auto rc0 = database::bind_text(stmthandle, 1, order_id);
	database::verify_binding(rc0);

	// Bind item_id
	const auto& item_id = g.first;
	auto rc1 = database::bind_text(stmthandle, 2, item_id);
	database::verify_binding(rc1);

	// Bind quantity
	const auto& quantity = g.second;
	auto rc2 = database::bind_int(stmthandle, 3, quantity);
	database::verify_binding(rc2);	
}

// status()
std::pair<bool, Order_Status_Type> order_manager::check_status(const std::string& order_id)
{
	// Validate order
	if (!exist(order_id))
	{
		return std::make_pair(false, Order_Status_Type::Invalid);
	}

	// Prepare SQL
	auto stmthandle 
		= order_statement_generator::query_order_status_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Bind SQL param
	auto rc = database::bind_text(*stmthandle, 1, order_id);
	database::verify_binding(rc);
	
	// Query
	rc = database::step(*stmthandle);
	database::verify_steping(rc);
	if (!database::step_has_result(rc))
	{
		throw warehouse_exception
		{
			"order_manager::check_status(): query return empty!"
		};
	}

	// Extract
	auto status_int = database::extract_int(*stmthandle, 0);
	auto status = int_to_status(status_int);
	return std::make_pair(true, status);
}
bool order_manager::update_status(const std::string& order_id, const Order_Status_Type nstatus)
{
	// Validate order
	if (!exist(order_id))
	{
		return false;
	}

	// Prepare SQL
	auto stmthandle 
		= order_statement_generator::update_status_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Bind SQL param:
	// Bind status
	auto s = status_to_int(nstatus);
	auto rc = database::bind_int(*stmthandle, 1, s);
	database::verify_binding(rc);

	// Bind order_id
	rc = database::bind_text(*stmthandle, 2, order_id);
	database::verify_binding(rc);

	// Step
	rc = database::step(*stmthandle);
	database::verify_steping(rc);
	
	return true;
}

// get_refund_order() and its helper functions
std::pair<bool, Refund_Order> order_manager::get_refund_order(const std::string& order_id)
{
	// Validate the order: existence and status
	if (!exist(order_id))		
	{
		return std::make_pair(false, Refund_Order{});
	}
	else if (Order_Status_Type::Refunded != check_status(order_id).second)
	{
		return std::make_pair(false, Refund_Order{});
	}
	else
	{
		// the order is validated
	}

	// Prepare SQL
	auto stmthandle = order_statement_generator::query_refund_order_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Bind SQL param
	auto rc = database::bind_text(*stmthandle, 1, order_id);
	database::verify_binding(rc);
		
	// Extract the query
	auto refund_order = Refund_Order{ order_id };
	extract_refund_query(refund_order, *stmthandle);

	return std::make_pair(true, refund_order);
}
void order_manager::extract_refund_query(Refund_Order& refund_order, sqlite3_stmt* stmthandle)
{
	while (SQLITE_ROW == database::step(stmthandle))
	{
		auto item_id = database::extract_text(stmthandle, 0);
		auto refund_quantity = database::extract_int(stmthandle, 1);
		auto g = good{ item_id, refund_quantity };
		refund_order.refund_goods.push_back(g);
	}
	if (refund_order.refund_goods.empty())
	{
		throw warehouse_exception
		{
			"order_manager::extract_refund_query(): query return empty!"
		};
	}
}

int order_manager::order_count()
{
	// Prepare SQL
	auto stmthandle = order_statement_generator::count_order_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Execute SQL
	auto rc = database::step(*stmthandle);
	database::verify_steping(rc);
	if (!database::step_has_result(rc))
	{
		throw warehouse_exception
		{
			"order_manager::order_count(): query return empty!"
		};
	}

	// Extract result
	auto count = database::extract_int(*stmthandle, 0);
	return count;
}