#include "warehouse.hpp"
#include <cstdio>
//--------
// shelf |
//--------
const std::string shelf::slot_indexes[] =
{
	"1", "2", "3", "4",
	"5", "6", "7", "8",
	"9", "10", "11", "12"
};
void shelf::initialize_slots()
{
	for (auto& id : slot_indexes)
	{
		slots.push_back(slot{});
	}
}


//------------
// warehouse |
//------------
void warehouse::load_warehouse(const std::vector<item_t>& items)
{
	for (auto& i : items)
	{
		// Extract item infos
		auto& item_id = std::get<id_type>(i);
		auto& count = std::get<unsigned>(i);
		auto& location = std::get<location_type>(i);
		auto& shelf_id = std::get<id_type>(location);
		auto slot = std::get<index_type>(location);
		std::cout << item_id << " "
			<< count << " "
			<< shelf_id << slot
			<< "\n";
		// Load shelves
		try 
		{
			auto& shelf_ = shelves.at(shelf_id);
			(*shelf_)[slot - 1].item_id = item_id;
			(*shelf_)[slot - 1].stock_counts = count;

			// Load location_by_itemid;
			auto location = std::make_tuple(shelf_id, slot);
			auto pair = std::make_pair(item_id, location);
			location_by_itemid.insert(std::move(pair));
		}
		catch (std::exception& e)
		{
			fprintf(stderr, "%s\n", e.what());
		}
	}
}

const std::vector<id_type> warehouse::shelves_ids =
{
	"A", "B", "C", "D"
};

void warehouse::initialize_shelves()
{
	for (auto& id : shelves_ids)
	{
		auto shelf_ = std::make_unique<shelf>(shelf{ id });
		this->shelves.insert({ id, std::move(shelf_) });
	}
}

std::tuple<bool, location_type> warehouse::locate_item(const id_type& item_id)
{
	// Try to find the item by the item id
	auto it = location_by_itemid.find(item_id);

	// Tell if the item exists in current data
	bool exists =
		(it != location_by_itemid.end())
		? true
		: false;		

	// Return loation
	return exists
		? std::make_tuple(exists, it->second)
		: std::make_tuple(exists, nowhere);
}

std::tuple<bool, id_type> warehouse::get_item(const location_type& location)
{
	// Extract location infos
	auto& shelf_id = std::get<id_type>(location);
	auto slot_index = std::get<index_type>(location);

	// Check the location, and get item if the location is valid
	auto it = shelves.find(shelf_id);
	bool exists =
		(it != shelves.end())
		? true
		: false;
	auto item_id = exists
		? it->second->slots[slot_index - 1].item_id
		: std::string{ "" };

	// Return the result
	return std::make_tuple(exists, item_id);
}

// Relocate an existing item to another location.
// Return true if succeed.
//bool relocate_item(const id_type&, const location_type&);

// Place a new item into the given location of the warehouse.
// Return true if succeeds.
//bool place_item(const id_type&, const location_type&);

// Remove an item from the warehouse.
// Return ture if succeed.
//bool remove_item(const id_type&);

// Update the item's stock given by its id 
// Return true if the item exists and the new count of stock is different.
//bool update_stock(const id_type&, unsigned);

const location_type warehouse::nowhere
	= std::make_tuple(std::string{ "" }, 0);