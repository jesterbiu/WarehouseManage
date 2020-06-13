#include "item_manager.hpp"
using namespace warehouse;
//item_manager::
bool item_manager::add_item(const Item& pitem)
{
	// Validate input item
	if (!pitem) return false;

	// Prepare statement
	auto stmthandle = insert_stmt();
	if (!stmthandle)
	{
		return false;
	}

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
	if (SQLITE_DONE == step(*stmthandle))
	{		
		occupy_location(item.location);
		return true;
	}
	
	return false;
}

std::pair<bool, Item> item_manager::find_item(const std::string& id)
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
	auto stmthandle = location_query_stmt();
	if (!stmthandle)
	{
		Item item; // Invalid item
		return std::make_pair(false, item);
	}

	// Bind sql parameter to values
	auto rc1 = sqlite3_bind_int(*stmthandle, 1, location.shelf);
	auto rc2 = sqlite3_bind_int(*stmthandle, 2, location.slot);
	if (rc1 != SQLITE_OK || rc2 != SQLITE_OK)
	{
		std::cout << "check_location: bind failed!\n";
		Item item; // Invalid item
		return std::make_pair(false, item);
	}

	// Execute sql
	auto rc = step(*stmthandle);
	if (rc != SQLITE_DONE && rc != SQLITE_ROW)
	{
		std::cout << "check_location: step failed!\n";
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
		(char)sqlite3_column_int(stmthandle, 0),	// shelf id
		sqlite3_column_int(stmthandle, 1),			// slot index
	};

	// id
	size_t buf_sz = 100;
	auto src = (const char*)sqlite3_column_text(stmthandle, 2);		
	d.item_id = src;

	// stocks
	d.stocks = sqlite3_column_int(stmthandle, 3);
}

void item_manager::bind_insert(const Item& d, sqlite3_stmt* stmthandle)
{
	// Bind location
	auto rc1 = sqlite3_bind_int(stmthandle, 1, d.location.shelf);
	auto rc2 = sqlite3_bind_int(stmthandle, 2, d.location.slot);

	// Bind id
	const char* id = d.item_id.c_str();
	auto rc3 = sqlite3_bind_text(
		stmthandle,
		3,
		id,
		d.item_id.size(),	// the offset of '\0'
		nullptr				// no deleter
		);

	// Bind stocks
	auto rc4 = sqlite3_bind_int(stmthandle, 4, d.stocks);

	// Validate
	if (rc1 != SQLITE_OK
		|| rc2 != SQLITE_OK
		|| rc3 != SQLITE_OK
		|| rc4 != SQLITE_OK)
	{
		throw warehouse_except("bind_insert: bind failed!");
	}

}

int item_manager::step(sqlite3_stmt* stmthandle)
{
	auto rc = sqlite3_step(stmthandle);
	
	// Error
	if (rc != SQLITE_DONE && rc != SQLITE_ROW)
	{
		switch (rc)
		{
		case SQLITE_BUSY:
			throw warehouse_except("SQLITE_BUSY", rc);
		case SQLITE_ERROR:
			throw warehouse_except("SQLITE_ERROR", rc);
		case SQLITE_MISUSE:
			throw warehouse_except("SQLITE_MISUSE", rc);
		default:
			throw warehouse_except("OTH", rc);
		}
	}
	return rc;
}

std::pair<bool, statement_handle> item_manager::query_by_id(const std::string& id)
{
	// Prepare statement
	auto stmthandle = id_query_stmt();
	if (!stmthandle)
	{
		return std::make_pair(false, std::move(stmthandle));
	}

	// Bind sql parameter to input value
	auto rc = sqlite3_bind_text(
		*stmthandle, 1,
		id.c_str(),
		id.size(),	// offest of '\0'
		nullptr		// no deleter
		);
	if (rc != SQLITE_OK)
	{
		std::cout << "query_by_id: bind failed!";
		return std::make_pair(false, std::move(stmthandle));
	}

	// Execute sql
	rc = step(*stmthandle);
	if (rc != SQLITE_DONE && rc != SQLITE_ROW)
	{
		return std::make_pair(false, std::move(stmthandle));
	}
	else
	{
		return std::make_pair(true, std::move(stmthandle));
	}
}

bool item_manager::update_stocks(const std::string& id, int updated_stocks)
{
	// Prepare statement
	auto stmthandle = update_stocks_stmt();
	if (!stmthandle)
	{
		return false;
	}

	// Bind sql parameter to input value
	auto rc1 = sqlite3_bind_int(*stmthandle, 1, updated_stocks);
	auto rc2 = sqlite3_bind_text(
		*stmthandle, 2,
		id.c_str(),
		id.size(),	// offest of '\0'
		nullptr		// no deleter
		);
	if (rc1 != SQLITE_OK || rc2 != SQLITE_OK)
	{
		std::cout << "update_stocks: bind failed!";
		return false;
	}

	// Execute sql
	auto rc = step(*stmthandle);
	if (rc != SQLITE_DONE && rc != SQLITE_ROW)
	{
		return false;
	}
	else
	{
		return true;
	}
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
	auto rc = sqlite3_exec(**db, sqlstmt, extract_locs_callback, &locs, &errmsg);

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
		throw warehouse_except(errmsg, rc);
	}
	
	return locs;
}

void item_manager::initialize_locations_avail()
{
	// shelf and slot designs
	static const char shelves[] = { 'A', 'B', 'C', 'D' };
	static const int slot_count = 12;

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