#include "order_test.hpp"
#include <map>

using namespace warehouse;

namespace Tests 
{
	void order_test()
	{
		try
		{
			// read
			std::map<std::string, Order> mo;
			storage::read_order_table(".\\Data\\order_tab_1.csv", mo);

			// prepare order_manager
			auto db = database::get_instance();
			auto om = order_manager{ db };

			for (auto& i : mo)
			{
				std::cout
					<< "order: "
					<< i.first
					<< ": ";

				// add
				if (om.add_order(i.second))
				{
					std::cout << "ADDED ";
				}
				else return;

				// check existence
				if (om.exist(i.first))
				{
					std::cout << "EXIST ";
				}
				else return;

				// get
				auto gotten = om.get_order(i.first);
				if (gotten.first && gotten.second == i.second)
				{
					std::cout << "GET\n";
				}
				else return;
			}
		}
		catch (warehouse_exception& we)
		{
			std::cout << we;
		}
	}
}
