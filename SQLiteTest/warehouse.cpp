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

	// Generate job for pickers

}
