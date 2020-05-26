// TEST.HPP
// THE FOLLOWING CODES ARE THE TEST FOR THE PROJECT

#pragma once
#include <fstream>
#include <algorithm>
#include <string_view>
#include <cstdio>
#include <iostream>
#include "warehouse_manager.hpp"


// Data source
std::string filepath = "..\\data\\items.txt";

// Read items info from .txt data source 
// And store item_t in vi
typedef std::tuple<location_type, id_type, unsigned> item_t;

void print_item_t(const item_t& i)
{
	auto& location = std::get<location_type>(i);
	auto& shelf_id = std::get<id_type>(location);
	auto& slot = std::get<index_type>(location);
	auto& item_id = std::get<id_type>(i);
	auto& count = std::get<unsigned>(i);
	std::cout << shelf_id << slot << "\t\t"
		<< item_id << "\t"
		<< count << "\n";
	
}

void read_item_data(std::vector<item_t>& vi)
{
	// Open data source
	std::ifstream data_src(filepath);

	// Initialize buffers to store item info
	char buf[100] = { 0 };
	char shelf_id[10] = { 0 };
	char item_id[50] = { 0 };
	auto slot = 0u, count = 0u;

	// Read items from data source
	while (!data_src.eof())
	{
		// Read line
		data_src.getline(buf, sizeof(buf));

		// Get formatted input from the line just read out
		// And return the number of paramaters read successfully
		auto reads = sscanf_s(buf,
			"%s\t%u\t%s\t%u",
			shelf_id, sizeof(shelf_id),
			&slot,
			item_id, sizeof(item_id),
			&count);

		// Discard this line if not four parameters were all read out
		if (reads != 4)
		{
			continue;
		}

		// Organize and store infos
		auto local_info = std::make_tuple(id_type(shelf_id), slot);
		auto item_info = std::make_tuple(local_info, id_type(item_id), count);
		vi.push_back(item_info);
	} // end of while

	// Close data source
	data_src.close();
}

void test_item_database()
{
	// Get item data
	std::vector<item_t> vi;
	read_item_data(vi);

	// Organize data
	std::vector<id_type> vif;
	for (auto& info : vi)
	{
		auto i = std::get<id_type>(info);
		vif.push_back(i);
	}

	// Load item_database
	auto& database = item_database::get_instance();
	database.load_item(vif);

	// Test get_item
	bool error = false;
	for (auto& id : vif)
	{		
		auto wptr = database.get_item(id);
		auto sptr = wptr.lock();
		if (sptr->get_id() != id)
		{
			error = true;
			std::cout << "Error: ";
			std::cout << sptr->get_id() << " vs " << id << "\n";
		}			
	}

	if (!error)
	{
		std::cout << "get_item test: pass\n";
	}
	else
	{
		std::cout << "get_item test: fail\n";
	}

}



