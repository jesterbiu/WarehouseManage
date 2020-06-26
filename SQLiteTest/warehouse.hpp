#pragma once
#include <memory>
#include <functional>
#include <iostream>
#include "storage.hpp"
#include "item_manager.hpp"
#include "order_manager.hpp"
#include "personnel_manager.hpp"
#define WAREHOUSE_ 1
#if WAREHOUSE_
namespace WarehouseManage
{		
	// warehouse
	class warehouse
	{
	public:
		// Ctor
		warehouse() :
			pdb(database::get_instance()),
			imng(nullptr), omng(nullptr), pmng(nullptr)
		{
			if (!pdb)
			{
				throw warehouse_exception
				{
					"warehouse(): failed to get database instance!"
				};
			}
			/*
			imng = std::make_unique<item_manager>(pdb);
			omng = std::make_unique<order_manager>(pdb);
			pmng = std::make_unique<personnel_manager>(pdb);
			*/
		}
		
		// Order picking
	public:
		// 生成任务 但不指定人员和时间执行
		std::pair<bool, std::string> pick(const Order&);			
		// Send the personnel's task queue to UI
		std::vector<const Task*> fetch_task_queue(const std::string& pers_id);
		// Return an operatable Task
		Task* fetch_task(const std::string& pers_id, const Task* pt)
		{
			if (!pt) return nullptr;
			else return pmng->fetch_task(pers_id, pt);
		}
		// Finish the task after the user performed
		void finish_task(const std::string& pers_id, Task* pt);

		void refund(const Refund_Order&);		

		// 生成任务，可以指定人员或时间执行
		struct Stock_Record
		{

		};
		Stock_Record check_stock(const Personnel&, const Location&);

	public:
		database* pdb;
		std::unique_ptr<order_manager>		omng;
		std::unique_ptr<item_manager>		imng;
		std::unique_ptr<personnel_manager>	pmng;		
						
		void finish_picking_task(Picking_Task* task);
		// Write inventory report to file
		void finish_inventory_task(Inventory_Task* task);
	};
}
#endif