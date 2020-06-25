#pragma once
#include <memory>
#include <functional>
#include "storage.hpp"
#include "item_manager.hpp"
#include "order_manager.hpp"
#include "personnel_manager.hpp"
#define WAREHOUSE_ 1
#if WAREHOUSE_
namespace WarehouseManage
{	
	class warehouse
	{
	public:
		// Ctor
		warehouse() :
			pdb(database::get_instance())
		{
			if (!pdb)
			{
				throw warehouse_exception
				{
					"warehouse(): failed to get database instance!"
				};
			}
			imng = std::make_unique<item_manager>(pdb);
			omng = std::make_unique<order_manager>(pdb);
			pmng = std::make_unique<personnel_manager>(pdb);
		}

		// 生成任务 但不指定人员和时间执行
		std::pair<bool, std::string> pick(const Order&);
		void refund(const Refund_Order&);		

		// 生成任务，可以指定人员或时间执行
		struct Stock_Record
		{

		};
		Stock_Record check_stock(const Personnel&, const Location&);

	private:
		database* pdb;
		std::unique_ptr<order_manager>		omng;
		std::unique_ptr<item_manager>		imng;
		std::unique_ptr<personnel_manager>	pmng;		
	};
}
#endif