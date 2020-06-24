#pragma once
#include <memory>
#include <functional>
#include "storage.hpp"
#include "item_manager.hpp"
#include "order_manager.hpp"
#include "personnel_manager.hpp"
#define WAREHOUSE_ 0
#if WAREHOUSE_

namespace WarehouseManage
{	
	class warehouse
	{
	public:
		//

		// 不指定人员
		void select(const Order&);
		void refund(const Refund_Order&);		

		// 指定人员
		struct Stock_Record
		{

		};
		Stock_Record check_stock(const Personnel&, const Location&);

	private:
		/*
		std::unique_ptr<order_manager>		om;
		std::unique_ptr<item_manager>		im;
		std::unique_ptr<personnel_manager>	pm;
		
		std::queue<Order>					unselected_orders;
		*/
	};

	
}
#endif