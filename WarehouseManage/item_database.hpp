#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include "item.hpp"

typedef std::shared_ptr<item> item_sptr;
typedef std::weak_ptr<item> item_wptr;

class item_database
{
public:
	static item_database& get_instance()
	{
		static auto instance = item_database();
		return instance;
	}

	// Load items infomation from storage
	void load_item(const std::vector<id_type>& vi)
	{
		for (auto& info : vi)
		{
			// Discard if the item already exists			
			if (contains(info))
			{
				continue;
			}

			// Create and store new item into the database
			auto new_item = std::make_shared<item>(info);
			items_by_id.insert({ new_item->get_id(), new_item });
		}
	}

	// Return a vector contains ids of all exisiting items
	std::vector<id_type> get_item_list()
	{
		std::vector<id_type> id_list;
		for (auto it = items_by_id.begin(); it != items_by_id.end(); it++)
		{
			id_list.push_back(it->first);
		}
		return id_list;
	}

	// Delete an item given its id,
	// return true if succeed
	// And notify the warehouse
	bool delete_item(const id_type& id)
	{
		if (!contains(id))
		{
			return false;
		}

		// Erase if exists
		items_by_id.erase(id);
		return true;		
	}

	// Add an item to the database, return true if succeed
	// And notify the warehouse
	bool add_item(const item& i)
	{
		// Check if the item already exists
		auto id = i.get_id();
		if (contains(id))
		{
			return false;
		}
		
		// Add in if not
		auto new_item = std::make_shared<item>(i);
		items_by_id.insert({ id, new_item });
		return true;		
	}

	
	// Check if the database contains the item given its id
	bool contains(const id_type& id)
	{
		// Try to find given id
		return items_by_id.find(id) == items_by_id.end()
			? false
			: true;
	}

	// Get an item given its id.
	// Return a weak_ptr to the item if it exists,
	// else return an empty weak_ptr
	item_wptr get_item(const id_type& id)
	{
		auto it = items_by_id.find(id);
		return it == items_by_id.end()
			? item_wptr()
			: item_wptr(it->second);
	}

private:
	// This unordered_map maintain all item objects using id as entry.
	// Every item is uniquely identified by its id.
	std::unordered_map<id_type, item_sptr> items_by_id;

	// Constructors
	item_database() {}
	item_database(const item_database&) = delete;
	item_database& operator= (const item_database&) = delete;
};