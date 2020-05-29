// TEST.HPP
// THE FOLLOWING CODES ARE THE TEST FOR THE PROJECT

#pragma once
#include "randomgenerator.hpp"
#include "warehouse_manager.hpp"


// Data source
const std::string filepath = "..\\data\\items.txt";

void print_item_t(const item_t& i);

std::string location_to_string(const location_type& location);

void read_item_data(std::vector<item_t>& vi);

void test_item_database();

void test_item_database_get_item(item_database& database, const std::vector<id_type>& vif);

void test_warehouse();

void test_warehouse_locate_item(warehouse& warehouse_, const std::vector<item_t>& vi);

void test_warehouse_get_item(warehouse& warehouse_, const std::vector<item_t>& vi);

void test_warehouse_update_stock(warehouse& warehouse_, const std::vector<item_t>& vi);
