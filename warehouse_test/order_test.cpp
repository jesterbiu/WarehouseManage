#include "order_test.hpp"


using namespace WarehouseManage;

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
			
			// get_all_order_ids
			auto count = (size_t)om.order_count();
			auto vog = std::vector<std::string>{ count };
			om.get_all_order_ids(vog.begin(), count);
			for (size_t i = 0; i < vog.size(); i++)
			{
				auto& id = vog[i];
				if (mo.find(id) == mo.end())
				{
					std::cout << "order_test: get_all_order_ids failed!";
					return;
				}
			}

			refund_order_test(mo, om);
		}
		catch (warehouse_exception& we)
		{
			std::cout << we;
		}
		std::cout << "order_test(): succeed\n\n";
	}
	void refund_order_test(std::map<std::string, Order>& mo, WarehouseManage::order_manager& om)
	{
		for (auto& i : mo)
		{
			const auto& order_id = i.first;
			std::cout << "refund_order_id: " << order_id;

			// update status
			if (om.update_status(order_id, Order_Status_Type::Refunded))
				std::cout << " UPDATED ";
			else { return;}

			// check status
			auto check = om.check_status(order_id);
			if (check.first
				&& check.second == Order_Status_Type::Refunded)
			{
				std::cout << "CHECKED ";
			}
			else { return; }

			// add refund_order
			auto rorder = Refund_Order{ order_id, i.second.goods };
			if (om.add_refund_order(rorder))
			{
				std::cout << "REFUNDED ";
			}
			else { return; }

			// get refund_order
			auto refund = om.get_refund_order(order_id);
			if (refund.first
				&& refund.second == rorder
				&& refund.second.refund_goods == rorder.refund_goods)
			{	
				std::cout << "GOT_REFUNDED\n";
			}
			else
			{ return; }
		} // end of for

		std::cout << "refund_order_test(): succeed\n\n";
	}
	bool operator==(const std::vector<good>& lhs, const std::vector<good>& rhs)
	{
		auto itl = lhs.cbegin();
		auto itr = rhs.cbegin();
		for (; itl != lhs.cend() && itr != rhs.cend(); itl++, itr++)
		{
			if (*itl != *itr)
			{
				return false;
			}
		}
		return true;
	}
}
