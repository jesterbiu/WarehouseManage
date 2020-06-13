#include "storage.hpp"


namespace warehouse {
// Utility function
// Split a string by the spliter given and return all fractions
std::vector<std::string> split(const std::string& str, char spliter)
{
	std::vector<std::string> vs;
	if (str.empty() || spliter == '\0')
	{
		return vs;
	}

	for (auto i1 = str.cbegin(); i1 != str.cend(); i1++)
	{
		// Look for spliter
		auto i2 = i1;
		for (; i2 != str.cend() && *i2 != spliter; i2++)
		{
		}

		// Split and store and substring
		vs.push_back(std::string(i1, i2));

		if (i2 != str.cend())
			i1 = i2;		// Continue
		else
			break;			// End fo the string
	}

	return vs;
}

void storage::read_item_table(const std::string& filename, std::vector<Item>& vi, Item_File_Type filetype)
{
	// Open file stream
	std::ifstream fs(filename);
	if (!fs.is_open())
	{
		throw warehouse_except("read_location_item_table: failed to open file!");
		return;
	}

	// Dispose the first line which is a comment
	//auto line = std::string();
	if (!fs.eof())
	{
		std::string s;
		std::getline(fs, s);
	}

	// Read file
	std::string line;
	while (!fs.eof())
	{
		std::getline(fs, line);
		auto vs = split(line, ',');		// Seperated columns by comma
		auto item = extract_item(vs, filetype);

		if (item)
		{
			vi.push_back(item);
		}
	}

	// Close stream
	fs.close();

}

Item storage::extract_item(const std::vector<std::string>& vi, Item_File_Type filetype)
{
	// Extract based on filetype
	switch (filetype)
	{
	case Item_File_Type::Located:
		if (vi.size() == 4)
		{
			return Item
			{
				Location {vi[0][0], std::stoi(vi[1])},	// location
				vi[2],									// item_id
				std::stoi(vi[3])						// stocks
			};
		}
		break;
	case Item_File_Type::NotLocated:
		if (vi.size() == 2)
		{
			return Item
			{
				Location(),								// location
				vi[0],									// item_id
				std::stoi(vi[1])						// stocks
			};
		}
		break;
	default:
		break;
	}

	// If filetype or count of fields not matched 
	// return an invalid item
	return Item();
}
}