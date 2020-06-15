#include "order_test.hpp"


using namespace warehouse;

namespace Tests 
{
	void order_test()
	{
		std::vector<Order> vo;
		storage::read_order_table("order_tab_1.csv", vo);
		for (auto& i : vo)
		{
			std::cout << i.order_id << "\n";
			for (auto& g : i.goods)
			{
				std::cout << "\t" 
					<< g.first << " " 
					<< g.second << "\n";
			}
		}
	}
}
