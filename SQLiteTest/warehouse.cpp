#include "warehouse.hpp"
using namespace WarehouseManage;

std::pair<bool, std::string> warehouse::pick(const Order& order)
{
	// Verify order
	if (!order && order.status != Order_Status_Type::PendForVerification)
	{
		std::string errmsg = "order" + order.order_id + " is not ready to be picked!";
		return std::make_pair(false, errmsg);
	}

	// Check stocks
	for (auto& g : order.goods)
	{
		// Fetch item info
		auto& item_id = g.first;
		auto result = imng->find_item(item_id);
		auto found = result.first;
		auto ordered_item = result.second;
		if (!found)
		{
			std::string errmsg = "item " + item_id + " does not exist!";
			return std::make_pair(false, errmsg);
		}

		// Veirfy item stock
		auto ordered_qty = g.second;
		if (ordered_qty > ordered_item.stocks)
		{
			std::string errmsg = "ordered quantity exceeds current stock!";
			return std::make_pair(false, errmsg);
		}

		// Update item stock
		auto updated_stock = ordered_item.stocks - ordered_qty;
		if (!imng->update_stocks(item_id, updated_stock))
		{			
			throw warehouse_exception
			{
				"failed to update item stocks!"
			};
		}

		// Update order status
		if (!omng->update_status(order.order_id, Order_Status_Type::PendForPicking))
		{
			// NEED TO WITHDRAW ITEM STOCK UPDATE
			throw warehouse_exception
			{ 
				"failed to update order status!"
			};
		}		
	}

	// Generate picking list
	auto picklist = std::vector<Picking_Info>{};
	for (auto& g : order.goods)
	{		
		
		auto& item_id = g.first;	

		auto qty = g.second;

		// Get item's location
		auto rt = imng->find_item(item_id);
		if (!rt.first) { return std::make_pair(false, "failed to find item!"); }
		auto location = rt.second.location;		
		
		// Store info
		picklist.emplace_back(Picking_Info{ item_id, qty, location });
	}

	// Generate task
	auto picktask = std::make_unique<Picking_Task>(order.order_id, picklist);
	return pmng->assign(std::move(picktask));	
}
std::vector<const Task*> warehouse::fetch_task_queue(const std::string& pers_id)
{
	auto vpt = std::vector<const Task*>{};
	if (pmng->find_personnel(pers_id).first == false)
	{
		return vpt;
	}
	 auto ptq = pmng->fetch_task_queue(pers_id);
	 for (auto& upt : *ptq)
	 {
		 const auto pt = upt.get();
		 vpt.push_back(pt);
	 }
	 return vpt;
}
void warehouse::finish_task(const std::string& pers_id, Task* pt)
{
	pmng->finish_task(pers_id, pt);
	switch (pt->tell_task_type())
	{
	case Task_Type::Picking_Type:
	{	
		auto pick = dynamic_cast<Picking_Task*>(pt);
		finish_picking_task(pick);
		return;
	}
	case Task_Type::Inventory_Type:
	{	
		auto invt = dynamic_cast<Inventory_Task*>(pt);
		//finish_inventory_task(invt);
		return;
	}
	default:
		break;
	} // end of switch
}
void warehouse::finish_picking_task(Picking_Task* task)
{
	auto& order_id = task->order_id;
	omng->update_status(order_id, Order_Status_Type::Picked);
}