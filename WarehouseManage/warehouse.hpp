#pragma once
#include <unordered_map>
#include <vector>
#include "item_database.hpp"

typedef unsigned index_type;
typedef std::tuple<id_type, index_type> location_type;

#define COMPILE 0

#if COMPILE

class slot
{
public:
	id_type item_id;
	unsigned stock_counts;
};


class shelf
{
	id_type shelf_id;
	std::vector<slot> slots;
};
typedef std::unique_ptr<shelf> shelf_uptr;

class warehouse
{
public:
	// Load the warehouse from storage
	void load_warehouse();

	// Return the location where the item is stored given its id. 
	// The bool returned indicates if the item is found.
	// Throw an exception if the item doesn't exists.
	std::tuple<bool, location_type> locate_item(const id_type&);

	// Relocate an existing item to another location.
	// Return true if succeed.
	bool relocate_item(const id_type&, const location_type&);

	// Place a new item into the given location of the warehouse.
	// Return true if succeeds.
	bool place_item(const id_type&, const location_type&);

	// Return the id of the item stored in the given slot location.
	// The bool returned indicates if the slot is empty.
	std::tuple<bool, id_type> get_item(const location_type&);

	// Remove an item from the warehouse.
	// Return ture if succeed.
	bool remove_item(const id_type&);

	// Update the item's stock given by its id 
	// Return true if the item exists and the new count of stock is different.
	bool update_stock(const id_type&, unsigned);

private:
	// This unordered_map maintain all shelf objects using id as entry.
	// Every shelf is uniquely identified by its id.
	static std::unordered_map<id_type, shelf_uptr> shelves;

	// This unordered_map maintain a mapping 
	// from an item id to its storing postion.
	static std::unordered_map<id_type, location_type> pos_by_itemid;
};

#endif