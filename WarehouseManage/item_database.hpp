#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include "item.hpp"

typedef std::shared_ptr<item> item_sptr; // smart ptr for item
typedef std::weak_ptr<item> item_wptr; // smart ptr for item

class item_database
{
public:
	static item_database& get_instance()
	{
		static auto instance = item_database();
		return instance;
	}

	// Load items infomation from storage
	void load_item(const std::vector<id_type>& vi);

	// Return a vector contains ids of all exisiting items
	std::vector<id_type> get_item_list();

	// Delete an item given its id,
	// return true if succeed
	// And notify the warehouse
	bool delete_item(const id_type& id);

	// Add an item to the database, return true if succeed
	// And notify the warehouse
	bool add_item(const item& i);
	
	// Check if the database contains the item given its id
	inline bool contains(const id_type& id)
	{
		// Try to find given id
		return items_by_id.find(id) == items_by_id.end()
			? false
			: true;
	}

	// Get an item given its id.
	// Return a weak_ptr to the item if it exists,
	// else return an empty weak_ptr
	item_wptr get_item(const id_type& id);

private:
	// This unordered_map maintain all item objects using id as entry.
	// Every item is uniquely identified by its id.
	std::unordered_map<id_type, item_sptr> items_by_id;

	// Constructors
	item_database() {}
	item_database(const item_database&) = delete;
	item_database& operator= (const item_database&) = delete;
};