#include "warehouse_test.hpp"
namespace Tests 
{
	using namespace WarehouseManage;
	std::unique_ptr<item_manager> get_item_manager()
	{
        std::vector<Item> vi;
        // read file                
        storage::read_item_table(".\\Data\\item_tab_1.csv", vi, Item_File_Type::Located);

        // initialize a item_manager instance
        auto db = database::get_instance();
        auto mp = std::make_unique<item_manager>(db);

        // insert items to database 
        int iter = 0;
        for (auto& i : vi)
        {
            // add_item
            if (!mp->add_item(i))
            {
                std::cout << "failed to ADD " << i.item_id << "\n";
            }
        }

        return mp;
	}
	std::unique_ptr<order_manager> get_order_manager()
	{
		// read
		std::map<std::string, Order> mo;
		storage::read_order_table(".\\Data\\order_tab_1.csv", mo);

		// prepare order_manager
		auto db = database::get_instance();
		auto om = std::make_unique<order_manager>(db);

		for (auto& i : mo)
		{
			std::cout
				<< "order: "
				<< i.first
				<< ": ";

			// add
			if (om->add_order(i.second))
			{
				std::cout << "ADDED ";
			}
			else return nullptr;
		}

		return om;
	}
	std::unique_ptr<personnel_manager> get_personnel_manager()
	{
		auto vp = std::vector<Personnel>{};

		storage::read_pers_table(".\\Data\\pers_tab_1.csv", vp);
		auto pdb = database::get_instance();
		auto pm = std::make_unique<personnel_manager>(pdb);

		for (auto& p : vp)
		{
			std::cout << p.personnel_id << ": ";

			if (pm->add_personnel(p))
			{
				std::cout << "ADDED ";
			}
			else return nullptr;
		}

		return pm;
	}
	void perform_picking(Picking_Task* pkt, const Order& order)
	{

	}
	void test_warehouse()
	{
		// initialize
		auto w = warehouse{};
		w.imng = get_item_manager();
		w.omng = get_order_manager();
		w.pmng = get_personnel_manager();

		// get an order
		auto sz = w.omng->order_count();
		auto ov = std::vector<Order>{ (size_t)sz };
		auto& order = ov[0];

		// record current item stocks
		auto sv = std::vector<int>{};
		for (auto& g : order.goods)
		{
			auto s = w.imng->find_item(g.first);
			if (!s.first) {
				std::cout << "find_item failed!";  return;
			}
			sv.push_back(s.second.stocks);
		}

		// assign a picking task
		auto prt = w.pick(order);
		if (prt.first == false) { std::cout << "pick() failed!"; return; }
		auto& pers_id = prt.second;

		// fetch task
		auto tq = w.fetch_task_queue(pers_id);
		auto t = w.fetch_task(pers_id, tq[0]);

		// perform picking task
		auto pkt = dynamic_cast<Picking_Task*>(t);
		perform_picking(pkt, order);

		// done
		w.finish_task(pers_id, t);
		
		// check by 
		// 1 fetching task queue again 
		tq = w.fetch_task_queue(pers_id);
		if (!tq.empty())
		{
			std::cout << "task queue not empty!\n";
			return;
		}
		// 2 verify order status
		auto update_o = w.omng->check_status(order.order_id);
		if (!update_o.first || update_o.second != Order_Status_Type::Picked)
		{
			std::cout << "order status update failed!\n";
			return;
		}
		// 3 verify items
		for (int i = 0; i != order.goods.size(); i++)
		{
			auto found = w.imng->find_item(order.goods[i].first);
			if (!found.first)
			{
				std::cout << "find_item() 2nd time failed\n";
				return;
			}
			auto currstock = found.second.stocks;
			if (sv[i] != order.goods[i].second + currstock)
			{
				std::cout << "update stock failed!";
				return;
			}
			
		}

		std::cout << "succeed\n";

	}
}

