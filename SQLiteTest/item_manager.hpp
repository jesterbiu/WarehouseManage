#pragma once
#include <memory>
#include <utility>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>
#include "error.hpp"
#include "database.hpp"
#include "item.hpp"

namespace warehouse
{
class item_manager
{	
private:
	// Database instance handle
	database* db;
public:
	// No default ctor
	// Param ctor
	item_manager(database* pdb) :
		db(pdb)
	{
		initialize_locations_avail();
	}
	// Copy ctor
	item_manager(const item_manager& oth) :
		db(oth.db), locations_avail(oth.locations_avail) {}
	// Copy-assignment ctor
	item_manager& operator =(const item_manager& oth)
	{
		if (this != &oth)
		{
			db = oth.db;
			locations_avail = oth.locations_avail;
		}
		return *this;
	}

	// Add a unique item to the database. Return true if success
	bool add_item(const Item& item);

	// Check if the item of given id exists. Return true if exists
	inline bool exist(const std::string& id)
	{
		auto result = query_by_id(id);
		return result.first;
	}

	// Find and return the item if exists. Test pair.first to validate the return
	std::pair<bool, Item> find_item(const std::string& id);
	
	// Check the location and return the item at the location if there is one.
	std::pair<bool, Item> check_location(const Location& location);

	// Update an item's stocks to currstock given its id
	bool update_stocks(const std::string& id, int currstock);

private:		
	// Return a statement_handle which selects item by id
	inline statement_handle id_query_stmt()
	{
		static auto sqlstmt =
			"SELECT shelf, slot, id, stocks FROM items WHERE id = $id_";
		return statement_handle{ sqlstmt, **db };
	}

	// Return a statement_handle which inserts an item
	inline statement_handle insert_stmt()
	{
		static auto sqlstmt =
			"INSERT INTO items(shelf, slot, id, stocks) VALUES($shelf, $slot, $id_, $stocks_)";
		return statement_handle{ sqlstmt, **db };
	}

	// Return a statement_handle which selects item by location
	inline statement_handle location_query_stmt()
	{
		static auto sqlstmt =
			"SELECT shelf, slot, id, stocks FROM items WHERE shelf = $shelf_ AND slot = $slot_";
		return statement_handle{ sqlstmt, **db };
	}

	// Return a statement_handle which uodate stocks by id
	inline statement_handle update_stocks_stmt()
	{
		static auto sqlstmt =
			"UPDATE items SET stocks = $stocks WHERE id = $id_";
		return statement_handle{ sqlstmt, **db };
	}

	// Bind data to write to (QUERY)
	void extract_query(Item& d, sqlite3_stmt* stmthandle);

	// Bind data to read from (INSERT, UPDATE)
	void bind_insert(const Item& d, sqlite3_stmt* stmthandle);
	
	// Fetch a row of data and print error if there is any
	int step(sqlite3_stmt* stmthandle);
	
	// Query by item id
	std::pair<bool, statement_handle> query_by_id(const std::string& id);

	// Val = true if the location is available for storing an item
	std::map<Location, bool> locations_avail;
	const bool Available = true;
	const bool Unavailable = false;
	
	// Return a vector contains all locations inserted
	std::vector<Location> get_unavail_locations();	
	
	// Initialize locations_avail during instantiation based on the database instance
	void initialize_locations_avail();

	// Check if the given locations is available, 
	// exception out_of_range is thrown if the location doesn't exists
	inline bool is_available(const Location& loc)
	{
		return locations_avail.at(loc);
	}
	
	inline void occupy_location(const Location& loc)
	{
		locations_avail.at(loc) = Unavailable;
	}

	// Return an available location.
	// The order of assignment of locations is unspecified.
	Location arrange_location();
};
}

