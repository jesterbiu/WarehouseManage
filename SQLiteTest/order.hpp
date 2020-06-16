#pragma once

#include <string>
#include <vector>

namespace warehouse {
	enum class Order_Status_Type
	{
		Invalid = 0,		// The order is not valid 
		PendForSelecting,	// The order just generated and goods are not selected
		Selected,			// Goods in the order are selected from the warehouse
		Refunded			// Refund is raised
	};
	
	// a pair of item_id and quantity
	using good = std::pair<std::string, int>;
	struct Order
	{
		// Default ctor
		Order() :
			status(Order_Status_Type::Invalid)
		{}
		// Param ctor
		Order(const std::string& id) :
			status(Order_Status_Type::PendForSelecting), order_id(id) {}
		// Copy ctor
		Order(const Order& oth) : 
			status(oth.status), order_id(oth.order_id), goods(oth.goods) {}
		// Copy-assignment
		Order& operator =(const Order& rhs)
		{
			if (this != &rhs)
			{
				status = rhs.status;
				order_id = rhs.order_id;
				goods = rhs.goods;
			}
			return *this;
		}
		// Default dotr
		~Order() {}

		explicit operator bool() const
		{
			return status != Order_Status_Type::Invalid;
		}

		bool operator ==(const Order& oth) const
		{
			return order_id == oth.order_id;
		}
		bool operator !=(const Order& oth) const
		{
			return order_id != oth.order_id;
		}

		// Fields
		Order_Status_Type status;
		std::string order_id;
		std::vector<good> goods; // items and quantities ordered by the customer
	};

	struct Refund_Order
	{
		// Default ctor
		Refund_Order() {}
		// Param ctor
		Refund_Order(const std::string& id, const std::vector<good>& vrg) :
			order_id(id), refund_goods(vrg) {}
		// Copy ctor
		Refund_Order(const Refund_Order& oth) :
			order_id(oth.order_id), refund_goods(oth.refund_goods) {}
		// Copy-assignment
		Refund_Order& operator =(const Refund_Order& rhs)
		{
			if (this != &rhs)
			{
				order_id = rhs.order_id;
				refund_goods = rhs.refund_goods;
			}
			return *this;
		}
		// Default dotr
		~Refund_Order() {}

		// Fields
		std::string order_id;
		std::vector<std::pair<std::string, int>> refund_goods; // item_id, quantity
	};
}