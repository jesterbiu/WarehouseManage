// Compilation dependency:
// ("a <- b" means b depends on a)
// pch.hpp 
// <- item.hpp 
// <- item_database.hpp
// <- warehouse.hpp
// <- warehouse_manager.hpp
// <- test.hpp
// <- main.cpp

#ifndef PCH_HPP
#define PCH_HPP

// STL utils
#include <unordered_map>
#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <algorithm>
#include <string_view>
#include <cstdio>
#include <iostream>

// Type aliases
typedef std::string id_type; // ID type of item and shelf
typedef unsigned index_type; // Index for slot of a shelf
typedef std::tuple<id_type, index_type> location_type; //
typedef std::tuple<location_type, id_type, unsigned> item_t; // Contains all infos about an item
typedef std::tuple<unsigned, location_type> item_info; // Stocks and location of an item
typedef std::tuple<id_type, unsigned> slot_info; // The stored item's id and its stocks of a slot


#endif



