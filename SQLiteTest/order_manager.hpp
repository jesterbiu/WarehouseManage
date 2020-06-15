#pragma once
#include <tuple>
#include "database.hpp"
#include "order.hpp"
namespace warehouse
{
	class order_manager
	{
	private:
		// Database instance handle
		database* db;
	public:
		// No default ctor
		// Param ctor
		order_manager(database* pdb) :
			db(pdb) {}
		// Copy ctor
		order_manager(const order_manager& oth) :
			db(oth.db) {}
		// Copy-assignments
		order_manager& operator =(const order_manager& rhs)
		{
			if (this != &rhs)
			{
				db = rhs.db;
			}
			return *this;
		}
		
		bool add_order(const Order& o);
		bool add_refund_order(const Refund_Order& ro);
		bool exist(const std::string& order_id);
		std::pair<bool, Order_Status_Type> check_status(const std::string& order_id);
		bool update_status(const std::string& order_id, const Order_Status_Type nstatus);
		std::pair<bool, Order> get_order(const std::string& order_id);
		std::pair<bool, Refund_Order> ger_refund_order();
	private:
	};
}