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
				std::cout << "ADDED\n";
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
				std::cout << "ADDED\n";
			}
			else return nullptr;
		}

		return pm;
	}
	void perform_picking(std::istream& is, Picking_Task* pkt, const Order& order)
	{
		std::cout << "Picking order " << order.order_id << "\n";
		std::cout << "Please input the last 6 character of item to confirm...\n";
		std::cout << "item_id\t\t\t\t\tquantity\tconfirm\n";
		auto idtail = std::make_unique<char[]>(7);
		for (auto& g : order.goods)
		{
			auto& itemid = g.first;
			std::cout << itemid << "\t" << g.second;
			if (is.eof()) return;
			is.get(idtail.get(), 7);

			auto sz = itemid.size();
			auto tail = itemid.substr(sz - 6);
			if (std::string{ idtail.get() } == tail)
			{
				std::cout << "\t\tY\n";
				continue;
			}
			else
			{
				std::cout << "bad input!\n";
			}
		}
		std::cout << "all picked and confirmed\n";
	}
	void test_warehouse()
	{
		// initialize
		auto w = warehouse{};
		w.item_mng = get_item_manager();
		w.order_mng = get_order_manager();
		w.personnel_mng = get_personnel_manager();

		// get an order
		auto sz = w.order_mng->order_count();
		auto ov = std::vector<std::string>{ (size_t)sz };
		w.order_mng->get_all_order_ids(ov.begin(), sz);// get order ids
		auto got = w.order_mng->get_order(ov[0]);// get the first order
		if (!got.first) return;
		auto& order = got.second;

		// record current item stocks
		auto sv = std::vector<int>{};
		for (auto& g : order.goods)
		{
			auto s = w.item_mng->find_item(g.first);
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
		std::string all_tails_str;
		for (auto& g : order.goods)
		{
			auto& id = g.first;
			all_tails_str += id.substr(id.size() - 6);
		}
		std::stringbuf s(all_tails_str);
		std::istream is(&s);
		perform_picking(is, pkt, order);

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
		auto update_o = w.order_mng->check_status(order.order_id);
		if (!update_o.first || update_o.second != Order_Status_Type::Picked)
		{
			std::cout << "order status update failed!\n";
			return;
		}
		// 3 verify items
		for (int i = 0; i != order.goods.size(); i++)
		{
			auto found = w.item_mng->find_item(order.goods[i].first);
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

