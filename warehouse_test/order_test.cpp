#include "order_test.hpp"
#include <map>

using namespace warehouse;

namespace Tests 
{
	void order_test()
	{
		std::map<std::string, Order> mo;
		storage::read_order_table(".\\Data\\order_tab_1.csv", mo);
		for (auto& i : mo)
		{
			std::cout << i.first << "\n";
			for (auto& g : i.second.goods)
			{
				std::cout << "\t" 
					<< g.first << " " 
					<< g.second << "\n";
			}
		}
	}
}
