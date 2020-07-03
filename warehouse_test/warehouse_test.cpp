#include "warehouse_test.hpp"

namespace Tests 
{
	using namespace WarehouseManage;
	
	void perform_item_task(std::istream& is, Item_Task* pkt)
	{
		// Print header and tips
		switch (pkt->item_type)
		{
		case Item_Type::InStock:
			std::cout << "In-stock Task:\n";
			break;
		case Item_Type::ToBeShipped:
			std::cout << "Picking order " << pkt->order_id << "\n";
			break;
		case Item_Type::Refunded:
			std::cout << "Refunding order " << pkt->order_id << "\n";
			break;
		}		
		std::cout << "Please input the last 6 character of item to confirm...\n";
		std::cout << "item_id\t\t\t\t\tloc\tqty\tconfirm\n";

		auto idtail = std::make_unique<char[]>(7);
		for (auto& g : pkt->item_infos)
		{
			auto& itemid = g.item_id;
			std::cout << itemid << "\t" 
				<< g.location << "\t"
				<< g.quantity;
			if (is.eof()) return;
			is.get(idtail.get(), 7);

			auto sz = itemid.size();
			auto tail = itemid.substr(sz - 6);
			if (std::string{ idtail.get() } == tail)
			{
				std::cout << "\tY\n";
				continue;
			}
			else
			{
				std::cout << "bad input!\n";
			}
		}//end of for

		std::cout << "all confirmed\n";
	}
	
	warehouse::Stock_Record perform_inv(const std::vector<int>& actuals, Inventory_Task* pit)
	{
		std::cout << "Doing inventory\n";
		std::cout << "Please input actual stock count of each item.\n";
		std::cout << "item_id\t\t\t\t\tloc\texpected\tactual\n";
		auto& invinfos = pit->inventory_infos;
		auto abeg = actuals.cbegin();
		auto ibeg = invinfos.begin();
		while (abeg != actuals.cend() && ibeg != invinfos.end())
		{
			std::cout << ibeg->item_id << "\t" 
				<< ibeg->location << "\t"
				<< ibeg->expected_stock_count << "\t\t"
				<< (ibeg->actual_stock_count = *abeg) << "\n";
			abeg++; ibeg++;			
		}
		return warehouse::Stock_Record{ *pit };
	}
	void test_warehouse()
	{
		// initialize
		auto w = warehouse{};
		w.item_mng = get_item_manager();
		w.order_mng = get_order_manager();
		w.personnel_mng = get_personnel_manager();	
		test_instock(w);
		test_pick(w);
		test_inv(w);
	}
	void test_pick(warehouse& w)
	{
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
			auto s = w.item_mng->get_item(g.first);
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
		auto pkt = dynamic_cast<Item_Task*>(t);
		std::string all_tails_str;
		for (auto& g : order.goods)
		{
			auto& id = g.first;
			all_tails_str += id.substr(id.size() - 6);
		}
		std::stringbuf s(all_tails_str);
		std::istream is(&s);
		perform_item_task(is, pkt);

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
			auto found = w.item_mng->get_item(order.goods[i].first);
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
	void test_inv(warehouse& w)
	{
		// select loc
		std::vector<Location> locs{};
		for (int i = 1; i <= 12; i++)
		{
			locs.emplace_back('A', i);
			locs.emplace_back('B', i);
		}

		// assign
		auto inv = w.check_inventory(locs);
		if (!inv.first) { std::cout << "check_inv() failed"; return; }
		auto pers_id = inv.second;
		
		// get task
		auto tq = w.fetch_task_queue(pers_id);
		if (tq.empty()) { std::cout << "empty task queue"; return; }
		auto pt = w.fetch_task(pers_id, tq[0]);

		// generate actual stock
		// acutal_stocks -> inventory_infos
		auto pitask = dynamic_cast<Inventory_Task*>(pt);
		std::vector<int> actual_stocks{};
		for (auto& i : pitask->inventory_infos)
		{
			auto l = i.location;
			auto found = w.item_mng->check_location(l);
			if (!found.first) { continue; }
			if (0 == l.slot % 3)
			{
				actual_stocks.push_back(found.second.stocks + 1);
			}
			else if (0 == l.slot % 4)
			{
				actual_stocks.push_back(found.second.stocks - 1);
			}
			else
			{
				actual_stocks.push_back(found.second.stocks);
			}
		}
				
		// perform_inv
		
		auto sr = std::move(perform_inv(actual_stocks, pitask));
		w.finish_task(pers_id, pt);

		// check 
		auto m = sr.get_differences();		
		auto info_to_stock = [&actual_stocks, sr](const Inventory_Info& info) 
		{
			for (size_t i = 0; i < sr.inventory_task.inventory_infos.size(); i++)
			{
				if (sr.inventory_task.inventory_infos[i] == info)
				{
					return actual_stocks[i];
				}				
			}
			return -1;
		};
		std::cout << "item\t\t\t\t\tloc\tdiff\tconfirm\n";
		for (auto& i : m)
		{			
			auto& item = i.first;			
			std::cout << item.item_id << "\t"
				<< item.location << "\t"
				<< i.second << "\t";

			auto stock = info_to_stock(item);
			if (stock < 0) 
			{ std::cout << "loc_to_stock() failed"; return; }
			auto expected_diff = stock - item.expected_stock_count;
			if (expected_diff != i.second) 
			{ std::cout << "expected_stock failed!"; return; }
			std::cout << "Y\n";
		}
	}
	void test_instock(warehouse& w)
	{
		std::vector<Item> vni, vei;
		// read file to get new items           
		storage::read_item_table(".\\Data\\item_tab_2.csv", vni, Item_File_Type::NotLocated);
		// query to get existing items
		auto sz = w.item_mng->item_count();
		vei.resize(sz);
		w.item_mng->get_all_items(vei.begin(), sz);
		// add them up
		std::vector<Item> vi = vni;
		vi.insert(vi.end(), vei.begin(), vei.end());

		// Gen task
		auto in = w.in_stock(vi);
		if (!in.first) { std::cout << "in_stock failed\n"; return; }
		auto pid = in.second;

		// Fetch task
		auto tq = w.fetch_task_queue(pid);
		if (tq.empty()) { std::cout << "fetch_tq failed\n"; return; }
		auto pt = w.fetch_task(pid, tq[0]);
		auto pit = dynamic_cast<Item_Task*>(pt);

		// Prep input
		std::string all_tails_str;
		for (auto& g : pit->item_infos)
		{
			auto& id = g.item_id;
			all_tails_str += id.substr(id.size() - 6);
		}
		std::stringbuf s(all_tails_str);
		std::istream is(&s);

		// Perform task
		perform_item_task(is, pit);
		w.finish_task(pid, pt);

		// Validate
		for (auto& i : vi)
		{
			std::cout << i.item_id << ": ";
			auto found = w.item_mng->get_item(i.item_id);
			if (!found.first)
			{
				std::cout << "not found!";
				return;
			}			
			if (std::find(vni.begin(), vni.end(), i) != vni.end())
			{
				if (found.second.item_id != i.item_id
					|| found.second.stocks != i.stocks)
				{
					std::cout << "failed\n";
				}
			}
			else
			{
				if (found.second.item_id != i.item_id
					|| found.second.stocks != i.stocks * 2)
				{
					std::cout << "failed\n";
				}
			}
			std::cout << "pass\n";
		}// end of validation for
	}
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
}

