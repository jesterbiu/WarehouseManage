#pragma once
#include <memory>
#include <utility>
#include <cmath>
#include <cstring>
#include "error.hpp"
#include "database.hpp"
#include "item.hpp"

class manage
{	
public:
	manage(database* pdb) :
		db(pdb) {}
	manage(const manage& oth) :
		db(oth.db) {}
	manage& operator =(const manage& oth)
	{
		if (this != &oth)
		{
			db = oth.db;
		}
		return *this;
	}

	bool add_item(const Item& item);

	inline bool exist(const std::string& id)
	{
		auto result = query_by_id(id);
		return result.first;
	}

	std::pair<bool, Item> find_item(const std::string& id);
	
	std::pair<bool, Item> check_location(const Location& location);

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

	// Field
	database* db;	
};

