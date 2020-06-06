#pragma once
#include <string>

struct Location
{
	Location() :
		shelf(0), slot(-1)
	{}
	Location(char pShelf, unsigned pSlot) :
		shelf(pShelf), slot(pSlot)
	{}

	// Shelf identifier
	char shelf;

	// Slot index
	unsigned slot;
};

struct Item
{
	Item() : stocks(-1) {}
	Item(const Location& pLocation, const std::string& pId, unsigned pStocks) :
		location(pLocation), item_id(pId), stocks(pStocks) {}

	// Location where the item is stored
	Location location;

	// Item's primary key
	std::string item_id;

	// Current stocks
	unsigned stocks;

};