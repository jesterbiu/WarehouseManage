#pragma once
#include <memory>
#include <utility>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>
#include "manager.hpp"
#include "item.hpp"

namespace WarehouseManage {
class item_manager : public manager
{	
public:
	// No default ctor
	item_manager() = delete;
	// Param ctor
	item_manager(database* pdb) :
		manager(pdb)
	{
		initialize_locations_avail();
	}
	// Copy ctor
	item_manager(const item_manager& oth) :
		manager(oth), locations_avail(oth.locations_avail) {}
	// Copy-assignment
	item_manager& operator =(const item_manager& oth)
	{
		if (this != &oth)
		{
			manager::operator=(oth);
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
	
	// Return the total count of items
	int item_count();

	// Try to store all but max_count of items in the container 
	// which the iter belongs to
	template<typename InputIt>
	void get_all_items(InputIt iter, size_t max_count)
	{
		// Validate input
		if (max_count < 1)
		{
			return;
		}

		// Prepare SQL
		auto stmth = item_statement_generator::select_all_stmt(get_database());
		database::verify_stmt_handle(stmth);

		// Execute
		auto rc = database::step(*stmth);
		database::verify_steping(rc);

		// Extract	
		auto i = 0;
		while (database::step_has_result(rc) && i != max_count)
		{			
			// Extract current row
			auto item = Item{};
			extract_query(item, *stmth);
			*iter = item;

			// Try to fetch the next row
			auto rc =database::step(*stmth);
			database::verify_steping(rc);

			// Increment
			iter++;
			i++;
		}
	}

	// Check the location and return the item at the location if there is one.
	std::pair<bool, Item> check_location(const Location& location);

	// Update an item's stocks to currstock given its id
	bool update_stocks(const std::string& id, int updated_stock);

private:		
	struct item_statement_generator : public statement_generator
	{
		// Return a statement_handle which selects item by id
		inline static statement_handle id_query_stmt(sqlite3* db)
		{
			static auto sqlstmt =
				"SELECT * FROM items WHERE id = $id_";
			return statement_handle{ sqlstmt, db };
		}

		// Return a statement_handle which inserts an item
		inline static statement_handle insert_stmt(sqlite3* db)
		{
			static auto sqlstmt =
				"INSERT INTO items(shelf, slot, id, stocks) VALUES($shelf, $slot, $id_, $stocks_)";
			return statement_handle{ sqlstmt, db };
		}

		// Return a statement_handle which selects item by location
		inline static statement_handle location_query_stmt(sqlite3* db)
		{
			static auto sqlstmt =
				"SELECT shelf, slot, id, stocks FROM items WHERE shelf = $shelf_ AND slot = $slot_";
			return statement_handle{ sqlstmt, db };
		}

		// Return a statement_handle which uodate stocks by id
		inline static statement_handle update_stocks_stmt(sqlite3* db)
		{
			static auto sqlstmt =
				"UPDATE items SET stocks = $stocks WHERE id = $id_";
			return statement_handle{ sqlstmt, db };
		}
	
		inline static statement_handle count_item_stmt(sqlite3* db)
		{
			static auto sqlstmt
				= "SELECT COUNT(*) FROM items; ";
			return statement_handle{ sqlstmt, db };
		}

		inline static statement_handle select_all_stmt(sqlite3* db)
		{
			static auto sqlstmt
				= "SELECT * FROM items; ";
			return statement_handle{ sqlstmt, db };
		}
	};
	

	// Bind data to write to (QUERY)
	void extract_query(Item& d, sqlite3_stmt* stmthandle);

	// Bind data to read from (INSERT, UPDATE)
	void bind_insert(const Item& d, sqlite3_stmt* stmthandle);
	
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

