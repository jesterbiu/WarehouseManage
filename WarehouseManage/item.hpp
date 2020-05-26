#pragma once
#include <string>

typedef std::string id_type;
class item
{
public:
	// Constructors
	item() 
	{}
	item(const id_type& id) :
		item_id(id)
	{}
	item(id_type&& id) :
		item_id(id)
	{}
	item(const item& oth) 
	{
		if (this != &oth)
		{
			item_id = oth.item_id;
		}
	}
	inline item(item&& oth) noexcept :
		item_id(std::move(oth.item_id))
	{}

	// Operator=
	item& operator=(const item& oth)
	{
		if (this != &oth)
		{
			item_id = oth.item_id;
		}
		return *this;
	}

	// Operator==
	bool operator==(const item& oth)
	{
		return item_id == oth.item_id;
	}

	// Operator!=
	bool operator!=(const item& oth)
	{
		return item_id != oth.item_id;
	}

	// Return the item's id
	inline const id_type& get_id() const
	{
		return item_id;
	}

private:
	// The id of the item
	id_type item_id;
};

