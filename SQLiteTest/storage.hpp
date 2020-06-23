#pragma once
#include <vector>
#include <fstream>
#include <map>
#include "warehouse_exception.hpp"
#include "item.hpp"
#include "order.hpp"

namespace WarehouseManage
{
	// Located indicates that the table contains location of the item, NotLocated means not
	enum class Item_File_Type { Located = 4, NotLocated = 2 };

	// Utility function
	// Split a string by the spliter given and return all fractions
	std::vector<std::string> split(const std::string& str, char spliter);

	// Manage file storage
	class storage
	{
	public:
		// Read the item table from the file opened by filename, and store the Item objects in vi
		// filetype: Located indicates that the table contains location of the item, NotLocated means not
		static void read_item_table(const std::string& filename, std::vector<Item>& vi, Item_File_Type filetype);

		static void read_order_table(const std::string& filename, std::map<std::string, Order>& vo);

		template<typename ValType, typename FileType>
		static void read_table(const std::string& filename, std::vector<Item>& vi, FileType filetype)
		{
			// Open file stream
			std::ifstream fs(filename);
			if (!fs.is_open())
			{
				throw warehouse_exception("read_location_item_table: failed to open file!");
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
				auto val = extract_val<ValType, FileType>(vs, filetype);

				if (val)
				{
					vi.push_back(val);
				}
			}

			// Close stream
			fs.close();
		}

	private:
		static Item extract_located_item(const std::vector<std::string>& vi)
		{
			// Validate input
			if (vi.size() != 4)
			{
				return Item();
			}
			
			// Extract
			auto item =  Item
			{
				Location {vi[0][0], std::stoi(vi[1])},	// location
				vi[2],									// item_id
				std::stoi(vi[3])						// stocks
			};
			return item;
		}

		static Item extract_notlocated_item(const std::vector<std::string>& vi)
		{
			// Validate input
			if (vi.size() != 2)
			{
				return Item();
			}

			// Extract
			auto item = Item
			{
				Location(),								// location
				vi[0],									// item_id
				std::stoi(vi[1])						// stocks
			};
			return item;
		}

		// Extract item infos from vi according to filetype and return the extracted item object
		static Item extract_item(const std::vector<std::string>& vi, Item_File_Type filetype);

		inline static Order extract_order(const std::vector<std::string>& vs)
		{
			return Order{ vs[0] };
		}

		static std::pair<bool, good> extract_good(const std::vector<std::string>& vs)
		{
			if (vs.size() == 3)
			{
				return std::make_pair(true, good{ vs[1], std::stoi(vs[2]) });
			}
			else
			{ 
				return std::make_pair(false, good{ "", -1 });
			}
		}

		// Extract template
		template<typename ValType, typename FileType>
		static ValType extract_val(const std::vector<std::string>& vi, FileType filetype)
		{
			return ValType();
		}

		// Extract template specialization for Item
		template<>
		static Item extract_val<Item, Item_File_Type>
			(const std::vector<std::string>& vi, Item_File_Type filetype)
		{
			Item item;

			// Extract based on filetype
			switch (filetype)
			{
			case Item_File_Type::Located:
				item = extract_located_item(vi);
				break;
			case Item_File_Type::NotLocated:
				item = extract_notlocated_item(vi);
				break;
			default:
				break;
			}

			return item;
		}

	};
}
