#include "manage.hpp"

//manage::
bool manage::add_item(const Item& item)
{
	// Prepare statement
	auto stmthandle = insert_stmt();
	if (!stmthandle)
	{
		return false;
	}

	// Bind				
	bind_insert(item, *stmthandle);

	// Execute
	return SQLITE_DONE == step(*stmthandle)
		? true
		: false;
}

std::pair<bool, Item> manage::find_item(const std::string& id)
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

std::pair<bool, Item> manage::check_location(const Location& location)
{
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

void manage::extract_query(Item& d, sqlite3_stmt* stmthandle)
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

void manage::bind_insert(const Item& d, sqlite3_stmt* stmthandle)
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

int manage::step(sqlite3_stmt* stmthandle)
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

std::pair<bool, statement_handle> manage::query_by_id(const std::string& id)
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

bool manage::update_stocks(const std::string& id, int updated_stocks)
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