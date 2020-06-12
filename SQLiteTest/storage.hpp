#pragma once
#include <vector>
#include <fstream>
#include "error.hpp"
#include "item.hpp"

// Located indicates that the table contains location of the item, NotLocated means not
enum class Item_File_Type { Located = 4, NotLocated = 2 };

// Manage file storage
class storage
{
public:
	// "loc_item_tab.csv"
	// Read the item table from the file opened by filename, and store the Item objects in vi
	// filetype: Located indicates that the table contains location of the item, NotLocated means not
	static void read_item_table(const std::string& filename, std::vector<Item>& vi, Item_File_Type filetype);

private:
	// Extract item infos from vi according to filetype and return the extracted item object
	static Item extract_item(const std::vector<std::string>& vi, Item_File_Type filetype);

};
