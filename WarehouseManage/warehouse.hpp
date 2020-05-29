#pragma once

#include "item_database.hpp"

#define COMPILE_WAREHOUSE 1
#if COMPILE_WAREHOUSE

//----------------//
//      slot      //
//----------------//
class slot
{
public:		
	id_type item_id;
	unsigned stock_counts;
};


//----------------//
//      shelf     //
//----------------//
class shelf
{
public:
	shelf()
	{
		initialize_slots();
	}
	shelf(const id_type& id) :
		shelf_id(id)
	{
		initialize_slots();
	}

	// Return slot using 1-based index
	slot& operator[](index_type slot_index)
	{
		return slots[slot_index - 1];
	}
		
	id_type shelf_id;
	

	// Indexes of slots
	const static std::string slot_indexes[];
private:	
	// Initialize slots with pre-defined slot IDs
	void initialize_slots();
	std::vector<slot> slots;
};
typedef std::unique_ptr<shelf> shelf_uptr;// shelf smart pointer

//---------------------//
//      warehouse      //
//---------------------//
class warehouse
{
public:
	static warehouse& get_instance()
	{
		static auto instance = warehouse();
		return instance;
	}

	// Load the warehouse from storage
	void load_warehouse(const std::vector<item_t>& items);

	// Return the location where the item is stored given its id. 
	// The bool returned indicates if the item is found.
	// Throw an exception if the item doesn't exists.
	std::tuple<bool, location_type> locate_item(const id_type& item_id);

	// Return the id of the item stored in the given slot location.
	// Return: tuple<true, valid item id> if succeed
	std::tuple<bool, id_type> get_item(const location_type&);

	// Update the item's stock given by its id 
	// Return:
	//	<0> true if the item exists
	//	<1> the difference between pre-update and updated stock count
	std::tuple<bool, int> update_stock(const id_type&, unsigned);

#if FALSE
	// Relocate an existing item to another location.
	// Return true if succeed.
	bool relocate_item(const id_type&, const location_type&);

	// Place a new item into the given location of the warehouse.
	// Return true if succeeds.
	bool place_item(const id_type&, const location_type&);	

	// Remove an item from the warehouse.
	// Return ture if succeed.
	bool remove_item(const id_type&);

#endif

	// Location error indicator
	const static location_type nowhere;
	// IDs of shelves
	const static std::vector<id_type> shelves_ids;

private:
	// This unordered_map maintain all shelf objects using id as entry.
	// Every shelf is uniquely identified by its id.
	std::unordered_map<id_type, shelf_uptr> shelves;

	// This unordered_map maintain a mapping 
	// from an item id to its storing postion.
	std::unordered_map<id_type, location_type> location_by_itemid;

	// Constructor
	warehouse()
	{
		initialize_shelves();
	}
	warehouse(const warehouse&) = delete;
	warehouse& operator=(const warehouse&) = delete;

	// Initialize all shelves with pre-defined shelf IDs
	void initialize_shelves();
};

#endif