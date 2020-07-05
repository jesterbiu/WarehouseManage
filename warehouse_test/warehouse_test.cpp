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
			std::cout << "\nIn-stock Task:\n";
			break;
		case Item_Type::ToBeShipped:
			std::cout << "\nPicking order " << pkt->order_id << "\n";
			break;
		case Item_Type::Refunded:
			std::cout << "\nRefunding order " << pkt->order_id << "\n";
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
	}	
	void perform_inv(const std::vector<int>& actuals, Inventory_Task* pit)
	{
		std::cout << "\nDoing inventory\n";
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
		std::cout << "end of perform_inv()\n";
	}
	
	void test_warehouse()
	{
		// initialize
		warehouse w;
		add_items(w);
		add_orders(w);
		add_personnels(w);	

		// tests
		test_instock(w);
		test_pick(w);
		test_refund(w);
		//test_inv(w);
	}
	
	// 对添加到数据库中第一个订单拣货
	void test_pick(warehouse& w)
	{
		// get an order
		auto sz = w.order_count();
		auto ov = std::vector<std::string>{ (size_t)sz };
		w.get_all_order_ids(ov.begin(), sz);// get order ids
		auto got = w.get_order(ov[0]);// get the first order
		if (!got.first) return;
		auto& order = got.second;

		// record current item stocks
		auto sv = std::vector<int>{};
		for (auto& g : order.goods)
		{
			auto s = w.get_item(g.first);
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
		auto update_o = w.check_order_status(order.order_id);
		if (!update_o.first || update_o.second != Order_Status_Type::Picked)
		{
			std::cout << "order status update failed!\n";
			return;
		}
		// 3 verify items
		for (int i = 0; i != order.goods.size(); i++)
		{
			auto found = w.get_item(order.goods[i].first);
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

		std::cout << "test_pick: succeed\n";
	}
	// 盘点A1~12, B1~12；其中货架号整除3的，实际库存+1；货架号不整除3但整除4的，实际库存-1；其他不变
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
			auto found = w.check_item_location(l);
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
		perform_inv(actual_stocks, pitask);
		w.finish_task(pers_id, pt);

		// check record
		auto srs = w.get_stock_records();
		if (srs.empty()) { throw warehouse_exception{"stock_record failed!"}; }
		auto sr = std::move(srs[0]);
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
		std::cout << "test_inv(): succeed\n";
	}
	//每种商品进货当前存量的一倍
	void test_instock(warehouse& w)
	{
		std::vector<Item> vni, vei;
		// read file to get new items           
		storage::read_item_table(".\\Data\\item_tab_2.csv", vni, Item_File_Type::NotLocated);
		// query to get existing items
		auto sz = w.item_count();
		vei.resize(sz);
		w.get_all_items(vei.begin(), sz);
		// add them up
		std::vector<Item> vi = vni;
		vi.insert(vi.end(), vei.begin(), vei.end());

		// Gen task
		auto in = w.in_stock(vi);
		if (!in.first) { std::cout << "in_stock failed: " << in.second << "\n"; return; }
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
			auto found = w.get_item(i.item_id);
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
		std::cout << "test_instock: succeed\n";
	}
	// 把test_pick()中完成拣货的订单全部退货
	void test_refund(warehouse& w)
	{
		// get a picked order
		std::vector<std::string> pickedorders;
		w.get_order_ids_by_status(pickedorders, Order_Status_Type::Picked);
		if (pickedorders.empty())
		{
			throw warehouse_exception{
				"get_order() failed"
			};
		}
		auto o = pickedorders[0];
		auto got = w.get_order(o);
		if (!got.first)
		{
			throw warehouse_exception{ "get_order()" };
		}
		auto order = got.second;

		// generate Refund order
		Refund_Order ro{ order.order_id, order.goods };

		// record item stocks for further validation
		std::vector<int> prev_qtys;
		for (auto& g : order.goods)
		{
			auto fi = w.get_item(g.first);
			if (!fi.first) { throw warehouse_exception{ "get_item() failed" }; }
			prev_qtys.push_back(fi.second.stocks);
		}

		// Gen task
		auto re = w.refund(ro);
		if (!re.first) { std::cout << "refund() failed\n"; return; }
		auto pid = re.second;

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

		// validate
		// order status
		auto cs = w.check_order_status(order.order_id);
		if (!cs.first || cs.second != Order_Status_Type::Refunded) {
			throw warehouse_exception{ "check_order_status(): failed" };
		}
		std::cout << "order status validated\n";

		// item stocks
		for (int i = 0; i != order.goods.size(); i++)
		{
			auto fi = w.get_item(order.goods[i].first);
			if (!fi.first) { throw warehouse_exception{ "get_item() failed" }; }
			auto curr_qty = fi.second.stocks;
			auto refunded_qty = order.goods[i].second;
			if (curr_qty != refunded_qty + prev_qtys[i])
			{
				std::cout << "stocks failed";
				return;
			}
		}
		std::cout << "item stocks validated\n";
		std::cout << "test_refund(): succeed\n";
	}

	void add_items(warehouse& w)
	{
		std::vector<Item> vi;
		// read file                
		storage::read_item_table(".\\Data\\item_tab_1.csv", vi, Item_File_Type::Located);

		// insert items to database 
		int iter = 0;
		std::cout << "\nADDING ITEMS...\n";
		for (auto& i : vi)
		{
			std::cout << "item " << i.item_id << ": ";
			// add_item
			if (!w.add_item(i))
			{
				throw warehouse_exception{"add_items()"};
			}
			std::cout << "ADDED\n";
		}
		std::cout << "success\n";
	}
	void add_orders(warehouse& w)
	{
		// read
		std::map<std::string, Order> mo;
		storage::read_order_table(".\\Data\\order_tab_1.csv", mo);
		
		std::cout << "\nADDING ORDERS...\n";
		for (auto& i : mo)
		{
			std::cout
				<< "order: "
				<< i.first
				<< ": ";

			// add
			if (!w.add_order(i.second))
			{
				throw warehouse_exception{ "add_orders()" };
			}
			std::cout << "ADDED\n";
		}
		std::cout << "success\n";
	}
	void add_personnels(warehouse& w)
	{
		auto vp = std::vector<Personnel>{};
		storage::read_pers_table(".\\Data\\pers_tab_1.csv", vp);
		
		std::cout << "\nADDING PERSONNELS...\n";
		for (auto& p : vp)
		{
			std::cout << p.personnel_id << ": ";
			if (!w.add_personnel(p))
			{
				throw warehouse_exception{ "add_pers()" };
			}
			std::cout << "ADDED\n";
			
		}
		std::cout << "success\n";
	}
}

