#pragma once

#include <string>
#include <vector>

namespace warehouse {
	enum class Order_Status_Type
	{
		Invalid,		// The order is not valid 
		PendForSelecting,	// The order just generated and goods are not selected
		Selected,			// Goods in the order are selected from the warehouse
		Refunded			// Refund is raised
	};
	
	inline int status_to_int(Order_Status_Type status)
	{
		switch (status)
		{
		case Order_Status_Type::Invalid:
			return 0;
		case Order_Status_Type::PendForSelecting:
			return 1;
		case Order_Status_Type::Selected:
			return 2;
		case Order_Status_Type::Refunded:
			return 3;
		default:
			throw warehouse_exception("Unknown order status!");
			break;
		}
	}

	inline Order_Status_Type int_to_status(int status)
	{
		switch (status)
		{
		case 0:
			return Order_Status_Type::Invalid;
		case 1:
			return Order_Status_Type::PendForSelecting;
		case 2:
			return Order_Status_Type::Selected;
		case 3:
			return Order_Status_Type::Refunded;
		default:
			throw warehouse_exception("Unknown order status!");
			break;
		}
	}

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
			status(Order_Status_Type::Invalid), order_id(id) {}
		Order(const std::string& id, const std::vector<good>& vg) :
			status(Order_Status_Type::Invalid), order_id(id), goods(vg) 
		{
			if (goods.size() > 0)
			{
				status = Order_Status_Type::PendForSelecting;
			}
		}
		template<typename Container>
		Order(const std::string& id, const Container& c) :
			status(Order_Status_Type::Invalid), order_id(id), goods(c.begin(), c.end())
		{
			if (goods.size() > 0)
			{
				status = Order_Status_Type::PendForSelecting;
			}
		}
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

		// Check if the order is valid
		explicit operator bool() const
		{
			return !order_id.empty()
				&& status != Order_Status_Type::Invalid
				&& !goods.empty();
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
		Refund_Order(const std::string& id) :
			order_id(id) {}
		Refund_Order(const std::string& id, const std::vector<good>& vrg) :
			order_id(id), refund_goods(vrg) {}
		template<typename Container>
		Refund_Order(const std::string& id, const Container& c) :
			order_id(id), refund_goods(c.begin(), c.end())
		{
			if (refund_goods.size() <= 0)
			{
				throw warehouse_exception
				{
					"Refund_Order: no refund good!"
				};
			}
		}
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

		explicit operator bool() const
		{
			return !order_id.empty()
				&& !refund_goods.empty();
		}
		bool operator ==(const Refund_Order& rhs) const
		{
			return order_id == rhs.order_id;
		}

		// Fields
		std::string order_id;
		std::vector<good> refund_goods; // item_id, quantity
	};
}