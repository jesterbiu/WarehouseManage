#include "storage.hpp"


namespace WarehouseManage 
{	
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

	// Read item
	void storage::read_item_table(const std::string& filename, std::vector<Item>& vi, Item_File_Type filetype)
	{
		// Open file stream
		std::ifstream fs(filename);
		if (!fs.is_open())
		{
			throw warehouse_exception("read_item_table: failed to open file!");
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
	
	// Read order
	void storage::read_order_table(const std::string& filename, std::map<std::string, Order>& vo)
	{
		// Open file stream
		std::ifstream fs(filename);
		if (!fs.is_open())
		{
			throw warehouse_exception("read_order_table: failed to open file!");
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
		std::string line, order_id;
		while (!fs.eof())
		{			
			std::getline(fs, line);
			auto vs = split(line, ',');		// Seperated columns by comma

			if (vs.empty())
			{				
				continue;
			}
			
			// Extract a new order if the first column is not empty
			if (!vs[0].empty())
			{
				order_id = vs[0];
				vo.emplace(order_id, Order{ order_id });
				vo[order_id].status = Order_Status_Type::PendForSelecting;
			}

			// Continue to extract a good's info
			auto g = extract_good(vs);
			if (g.first)
			{
				if (order_id.empty())
				{
					throw warehouse_exception("storage::read_order_table: missing order id!");
				}
				vo[order_id].goods.push_back(g.second);
			}

		}

		// Close stream
		fs.close();
	}
	
	// Read personne;
	void storage::read_pers_table(const std::string& filename, std::vector<Personnel>& vp)
	{
		// Open file stream
		std::ifstream fs(filename);
		if (!fs.is_open())
		{
			throw warehouse_exception("read_order_table: failed to open file!");
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
			auto cols = split(line, ',');		// Seperated columns by comma

			// Validate every column
			if (cols.empty())
			{
				continue;
			}
			else
			{
				bool empty_col = false;
				for (auto& col : cols)
				{
					if (col.empty())
					{
						empty_col = true;
						break;
					}
				}
				if (empty_col) continue;
			}

			auto pers = Personnel{};
			// Role			
			switch (cols[0][0])
			{
			case 'A':
				pers.role = Personnel_Role::Admin;
				break;
			case 'M':
				pers.role = Personnel_Role::Manager;
				break;
			case 'P':
				pers.role = Personnel_Role::Picker;
				break;
			default:
				break;
			}

			// ID
			pers.personnel_id = cols[1];

			// Password
			pers.password = cols[2];

			vp.push_back(pers);		
		}

		// Close stream
		fs.close();
	}

}