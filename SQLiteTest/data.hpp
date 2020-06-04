#pragma once
#include <string>
struct data
{
	data() : stocks(0) {}	// Construct an empty data object
	data(const std::string& id_, const std::string& loc_, unsigned stocks_) :
		ID(id_), loc(loc_), stocks(stocks_) {}

	std::string ID;
	std::string	loc;
	unsigned	stocks;	
};