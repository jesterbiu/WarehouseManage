#pragma once
#include <string>
#include <ostream>
namespace WarehouseManage
{ 
struct Location
{
	// Defualt ctor
	Location() :
		shelf(0), slot(-1)
	{}
	// Custom param ctor
	Location(char pShelf, int pSlot) :
		shelf(pShelf), slot(pSlot)
	{}
	// Copy ctor
	Location(const Location& oth) :
		shelf(oth.shelf), slot(oth.slot) {}	
	// Copy-assignment
	Location& operator =(const Location& oth)
	{
		if (this != &oth)
		{
			shelf = oth.shelf;
			slot = oth.slot;
		}
		return *this;
	}
	// Default ctor
	~Location() {}

	// Return true if the location is valid
	explicit operator bool() const
	{
		return std::isalpha(shelf) && (slot >= 0);
	}

	// Relatioship operators
	bool operator ==(const Location& oth) const
	{
		return oth.shelf == shelf 
			&& oth.slot == slot;
	}

	bool operator <(const Location& rhs) const
	{
		if (shelf < rhs.shelf)
		{
			return true;
		}
		else if (shelf == rhs.shelf)
		{
			return slot < rhs.slot;
		}
		else // shelf > rhs.shelf
		{
			return false;
		}		
	}
	
	// Shelf identifier
	char shelf;

	// Slot index
	int slot;
};
std::ostream& operator <<(std::ostream& os, const Location& loc);
struct Item
{
	// Default ctor
	Item() : stocks(-1) {}
	// Custom param ctor
	Item(const Location& pLocation, const std::string& pId, int pStocks) :
		location(pLocation), item_id(pId), stocks(pStocks) {}
	// Copy ctor
	Item(const Item& oth) :
		location(oth.location), item_id(oth.item_id), stocks(oth.stocks) {}
	// Copy-assignment
	Item& operator =(const Item& oth)
	{
		if (this != &oth)
		{
			location = oth.location;
			item_id = oth.item_id;
			stocks = oth.stocks;
		}
		return *this;
	}
	// Default dtor
	~Item() {}

	// Valid test
	explicit operator bool() const
	{
		return 
			!item_id.empty() && stocks >= 0;
	}

	// Equality
	bool operator ==(const Item& oth) const
	{
		return oth.location == location
			&& oth.item_id == item_id
			&& oth.stocks == stocks;
	}
	bool operator !=(const Item& oth) const
	{
		return !operator== (oth);
	}

	// Location where the item is stored
	Location location;

	// Item's primary key
	std::string item_id;

	// Current stocks
	int stocks;

};
}