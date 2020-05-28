#include "item_database.hpp"

void item_database::load_item(const std::vector<id_type>& vi)
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

std::vector<id_type> item_database::get_item_list()
{
	std::vector<id_type> id_list;
	for (auto it = items_by_id.begin(); it != items_by_id.end(); it++)
	{
		id_list.push_back(it->first);
	}
	return id_list;
}

bool item_database::delete_item(const id_type& id)
{
	if (!contains(id))
	{
		return false;
	}

	// Erase if exists
	items_by_id.erase(id);
	return true;
}

bool item_database::add_item(const item& i)
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



item_wptr item_database::get_item(const id_type& id)
{
	auto it = items_by_id.find(id);
	return it == items_by_id.end()
		? item_wptr()
		: item_wptr(it->second);
}
