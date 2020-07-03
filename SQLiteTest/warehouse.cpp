#include "warehouse.hpp"
using namespace WarehouseManage;

// Fetching and finishing tasks
std::vector<const Task*> warehouse::fetch_task_queue(const std::string& pers_id)
{
	auto vpt = std::vector<const Task*>{};
	if (personnel_mng->find_personnel(pers_id).first == false)
	{
		return vpt;
	}
	 auto ptq = personnel_mng->fetch_task_queue(pers_id);
	 for (auto& upt : *ptq)
	 {
		 const auto pt = upt.get();
		 vpt.push_back(pt);
	 }
	 return vpt;
}
void warehouse::init_finishers()
{
	auto itemio = [this](Task* pt) { finish_itemio_task(pt); };
	auto inventory = [this](Task* pt) { finish_inventory_task(pt); };
	task_finisher =
	{
		{
			Task_Type::ItemIO_Type, 
			std::function<void(Task*)>(itemio)
		},
		{
			Task_Type::Inventory_Type,
			std::function<void(Task*)>(inventory)
		}
	};	

	auto instock = [this](Item_Task* it) { finish_instock_task(it); };
	auto pick = [this](Item_Task* it) { finish_picking_task(it); };
	auto refund = [this](Item_Task* it) { finish_refund_task(it); };
	item_task_finisher =
	{
		{
			Item_Type::InStock, 
			std::function<void(Item_Task*)>(instock)
		},
		{
			Item_Type::ToBeShipped,
			std::function<void(Item_Task*)>(pick)
		},
		{
			Item_Type::Refunded,
			std::function<void(Item_Task*)>(refund)
		}
	};
}
void warehouse::finish_task(const std::string& pers_id, Task* pt)
{	
	/*
	switch (pt->tell_task_type())
	{
	case Task_Type::ItemIO_Type:
	{			
		//finish_itemio_task(pick);
		break;
	}
	case Task_Type::Inventory_Type:
	{	
		auto invt = dynamic_cast<Inventory_Task*>(pt);
		//finish_inventory_task(invt);
		break;
	}
	default:
		break;
	} // end of switch
	*/
	task_finisher[pt->tell_task_type()](pt);
	// Delete task
	personnel_mng->finish_task(pers_id, pt);
}
void warehouse::finish_itemio_task(Task* task)
{
	auto pit = dynamic_cast<Item_Task*>(task);
	item_task_finisher[pit->item_type](pit);
}
void warehouse::finish_picking_task(Item_Task* task)
{
	auto& order_id = task->order_id;
	order_mng->update_status(order_id, Order_Status_Type::Picked);
}
void warehouse::add_stocks(const std::vector<Item_Info>& infos) 
{
	for (auto& i : infos)
	{
		auto got = item_mng->get_item(i.item_id);
		if (!got.first)
		{
			throw warehouse_exception
			{
				"finish_refund_task(): item not foound!"
			};
		}
		auto updated_stocks = got.second.stocks + i.quantity;
		item_mng->update_stocks(i.item_id, updated_stocks);
	}
}

// pick() and its helper functions
std::pair<bool, std::string> warehouse::verify_ordered_goods(const Order& order)
{
	// Check stocks
	for (auto& g : order.goods)
	{
		// Fetch item info
		auto& item_id = g.first;
		auto result = item_mng->get_item(item_id);
		if (!result.first)
		{
			std::string errmsg = "item " + item_id + " does not exist!";
			return std::make_pair(false, errmsg);
		}

		// Veirfy item stock
		auto ordered_qty = g.second;
		auto ordered_item = result.second;
		if (ordered_qty > ordered_item.stocks)
		{
			std::string errmsg = "ordered quantity exceeds current stock!";
			return std::make_pair(false, errmsg);
		}

		// Update item stock
		auto updated_stock = ordered_item.stocks - ordered_qty;
		if (!item_mng->update_stocks(item_id, updated_stock))
		{
			throw warehouse_exception
			{
				"failed to update item stocks!"
			};
		}		
	} // end of for
	return std::make_pair(true, std::string{});
}
std::vector<Item_Info> warehouse::get_good_info_list(const std::vector<good>& goods)
{
	auto picklist = std::vector<Item_Info>{};
	for (auto& g : goods)
	{

		auto& item_id = g.first;

		auto qty = g.second;

		// Get item's location
		auto rt = item_mng->get_item(item_id);
		// Throw if the item is not found 
		// since verify_ordered_goods() just checked that
		if (!rt.first)  
		{
			throw warehouse_exception
			{
				"get_picking_list(): item not found!"
			};
		}
		auto location = rt.second.location;

		// Store info
		picklist.emplace_back(Item_Info{ item_id, qty, location });
	}
	return picklist;
}
std::pair<bool, std::string> warehouse::pick(const Order& order)
{
	// Verify order
	if (!order || order.status != Order_Status_Type::PendForVerification)
	{
		std::string errmsg = "order is not ready to be picked!";
		return std::make_pair(false, errmsg);
	}
	else if (!order_mng->exist(order.order_id))
	{
		std::string errmsg = "order does not exist!";
		return std::make_pair(false, errmsg);
	}
	else {}

	// Verify ordered goods and update their stocks
	auto rt = verify_ordered_goods(order);
	if (!rt.first) { return rt; }
	
	// Update order status
	if (!order_mng->update_status(order.order_id, Order_Status_Type::PendForPicking))
	{
		// NEED TO WITHDRAW ITEM STOCK UPDATE
		throw warehouse_exception
		{ 
			"failed to update order status!"
		};
	}		
	
	// Generate picking list
	auto picklist = std::move(get_good_info_list(order.goods));

	// Generate task and assign
	auto picktask = std::make_unique<Item_Task>
		(order.order_id, picklist, Item_Type::ToBeShipped);
	return personnel_mng->assign(std::move(picktask));	
}

// refund() and its helper functions
std::pair<bool, std::string> warehouse::verify_refunded_goods(const Order& order, const Refund_Order& rorder)
{
	auto prev = order_mng->get_refund_order(order.order_id);
	for (auto& g : rorder.refund_goods)
	{
		// Verify that the refunded good was in the order list
		auto ordered = std::find(order.goods.begin(), order.goods.end(), g);
		if (ordered == order.goods.end())
		{
			auto errmsg = "a refunded good wasn't in the order list!";
			return std::make_pair(false, errmsg);
		}

		// Compare ordered quantity and refunded quantity
		auto r_qty = g.second, 
			prev_r_qty = 0,
			ordered_qty = ordered->second;
		// Get previous refunds if there is any
		if (prev.first)
		{
			auto prev_refunded = prev.second.refund_goods;
			auto prev_g = prev_refunded.begin();
			while (prev_refunded.end() 
				!= (prev_g = std::find(prev_g, prev_refunded.end(), g)))
			{
				prev_r_qty += prev_g->second;
				prev_g++;
			}
		}
		// Return false if refunded qty > ordered qty
		if (prev_r_qty + r_qty > ordered_qty)
		{
			auto errmsg = "refunded quantity exceeds ordered quantity!";
			return std::make_pair(false, errmsg);
		}
	} // end of for
	return std::make_pair(true, std::string{});
}
std::pair<bool, std::string> warehouse::refund(const Refund_Order& rorder)
{
	// Validate the refund order
	if (!rorder || !order_mng->exist(rorder.order_id))
	{
		auto errmsg = "invalid refund order!";
		return std::make_pair(false, errmsg);
	}

	// Fetch the original order
	auto found = order_mng->get_order(rorder.order_id);
	if (!found.first) 
	{ 
		auto errmsg = "order does not exist!";
		return std::make_pair(false, errmsg);
	}
	auto& order = found.second;

	// Verify refunded goods
	auto verified = verify_refunded_goods(order, rorder);
	if (!verified.first) { return verified; }

	// Update status
	if (Order_Status_Type::Refunded != order_mng->check_status(order.order_id).second)
	{
		auto updated = order_mng->update_status(order.order_id, Order_Status_Type::Refunded);
		if (!updated)
		{
			throw warehouse_exception
			{
				"refund(): update_status() failed!"
			};
		}
	}

	// Generate list for returning goods
	auto refund_list = std::move(get_good_info_list(rorder.refund_goods));

	// Generate task and assign
	auto refund_task = std::make_unique<Item_Task>
		(order.order_id, refund_list, Item_Type::Refunded);
	return personnel_mng->assign(std::move(refund_task));
}


std::map<Inventory_Info, int> warehouse::Stock_Record::get_differences()
{
	std::map<Inventory_Info, int> mii{};
	for (auto& i : inventory_task.inventory_infos)
	{
		// Ignore unchecked stock count
		if (i.actual_stock_count < 0) { continue; }

		// Get difference
		if (i.actual_stock_count != i.expected_stock_count)
		{
			mii.emplace(i, i.actual_stock_count - i.expected_stock_count);
		}
	}
	return mii;
}
