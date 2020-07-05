#include "item_manager.hpp"
using namespace WarehouseManage;
//item_manager::
bool item_manager::add_item(const Item& pitem)
{
	// Validate input item
	if (!pitem || exist(pitem.item_id)) return false;

	// Prepare statement
	auto stmthandle = item_statement_generator::insert_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Get a copy in case location allocation happens
	auto item = Item(pitem); 

	// Check item location
	if (item.location) // Check pre-assigned location
	{		
		// Return false if the item's location is already unavailable
		if (!is_available(item.location))
		{
			return false;
		}
	}
	else // Allocate a location for item that didn't
	{
		// Return false if no location available 
		if (!(item.location = arrange_location()))
		{
			return false;
		}		
	}

	// Bind SQL params
	bind_insert(item, *stmthandle);

	// Execute insertion
	if (SQLITE_DONE == database::step(*stmthandle))
	{		
		occupy_location(item.location);
		return true;
	}
	
	return false;
}

std::pair<bool, Item> item_manager::get_item(const std::string& id)
{
	// Check existence
	auto result = query_by_id(id);
	if (!result.first)
	{
		Item item; // Invalid item
		return std::make_pair(false, item);
	}

	// Extract results
	auto d = Item{};
	extract_query(d, *result.second);

	return std::make_pair(true, d);
}

std::pair<bool, Item> item_manager::check_location(const Location& location)
{
	// Validate input
	if (!location)
	{
		return std::make_pair(false, Item());
	};

	// Prepare statement
	auto stmthandle = item_statement_generator::location_query_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Bind SQL params:
	// Bind shelf:
	database::bind_int(*stmthandle, 1, location.shelf);

	// Bind slot:
	database::bind_int(*stmthandle, 2, location.slot);

	// Execute sql
	auto rc = database::step(*stmthandle);
	if (!database::step_has_result(rc))
	{
		Item item; // Invalid item
		return std::make_pair(false, item);
	}
		
	// Extract results
	auto d = Item{};
	extract_query(d, *stmthandle);

	return std::make_pair(true, d);
}

void item_manager::extract_query(Item& d, sqlite3_stmt* stmthandle)
{
	// Location
	d.location = Location
	{
		(char)database::extract_int(stmthandle, 0),		// shelf id
		database::extract_int(stmthandle, 1),			// slot index
	};

	// id
	d.item_id = database::extract_text(stmthandle, 2);

	// stocks
	d.stocks = database::extract_int(stmthandle, 3);
}

void item_manager::bind_insert(const Item& d, sqlite3_stmt* stmthandle)
{
	// Bind location
	database::bind_int(stmthandle, 1, d.location.shelf);
	database::bind_int(stmthandle, 2, d.location.slot);

	// Bind id
	database::bind_text(stmthandle, 3, d.item_id);

	// Bind stocks
	database::bind_int(stmthandle, 4, d.stocks);
}

std::pair<bool, statement_handle> item_manager::query_by_id(const std::string& id)
{
	// Prepare statement
	auto stmthandle = item_statement_generator::id_query_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Bind sql parameter to input value
	database::bind_text(*stmthandle, 1, id);

	// Execute sql
	auto rc = database::step(*stmthandle);
	if (database::step_has_result(rc))
	{
		return std::make_pair(true, std::move(stmthandle));
	}
	else
	{
		return std::make_pair(false, std::move(stmthandle));
	}
}

bool item_manager::update_stocks(const std::string& id, int updated_stocks)
{
	// Prepare statement
	auto stmthandle = item_statement_generator::update_stocks_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Bind SQL params:
	// Bind updated stocks
	database::bind_int(*stmthandle, 1, updated_stocks); 

	// Bind item_id
	database::bind_text(*stmthandle, 2, id);

	// Execute sql
	database::step(*stmthandle);
	return true;
}

int extract_locs_callback(
	void* vl,	// vector to store locations the query returns
	int col_count,				// the number of cloumns in the result
	char** col_values,			// value of each column
	char** col_names			// name of each column
	)
{
	// Extract a location
	if (vl && col_count == 2)
	{
		// get shelf
		char shelf = col_values[0][0];

		// get slot
		auto slot_str = std::string{ col_values[1] };
		int slot = std::stoi(slot_str);

		// record
		auto loc = Location{ shelf, slot };
		((std::vector<Location>*)vl)->push_back(loc);
		
		return 0;
	}

	return -1;	
}

std::vector<Location> item_manager::get_unavail_locations()
{
	// Vector to store locations
	std::vector<Location> locs;

	// Execute query and extract results
	static auto sqlstmt =
		"SELECT shelf, slot FROM items";
	char* errmsg = NULL;
	auto rc = sqlite3_exec(get_database(), sqlstmt, extract_locs_callback, &locs, &errmsg);

	// Validate execution
	if (rc != SQLITE_OK)
	{
		// throw
		std::cout << "get_unavail_locations failed:";
		if (errmsg)
		{
			std::cout << errmsg;
		}
		std::cout << std::endl;
		throw warehouse_exception(errmsg, rc);
	}
	
	return locs;
}

void item_manager::initialize_locations_avail()
{

	// Initialize all locations as available
	for (auto s : shelves)						// ['A', 'D']
	{
		for (int i = 1; i <= slot_count; i++)	// [1, 12]
		{
			auto loc = Location{ s, i };
			locations_avail.insert({ loc, Available });
		}
	}

	// Modify locations that are not available
	auto vl = get_unavail_locations();
	for (auto& l : vl)
	{
		locations_avail[l] = Unavailable;
	}
}

Location item_manager::arrange_location()
{
	// Traverse and return the first available location
	for (const auto& i : locations_avail)
	{
		if (i.second == Available)
		{
			return i.first;
		}
	}

	// Return an invalid location if no available location found
	return Location();
}

int item_manager::item_count()
{
	// Prepare SQL
	auto stmthandle = item_statement_generator::count_item_stmt(get_database());
	database::verify_stmt_handle(stmthandle);

	// Execute SQL
	auto rc = database::step(*stmthandle);
	if (!database::step_has_result(rc))
	{
		throw warehouse_exception
		{
			"item_manager::item_count(): query return empty!"
		};
	}

	// Extract result
	auto count = database::extract_int(*stmthandle, 0);
	return count;
}