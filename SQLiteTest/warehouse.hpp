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
			item_mng(nullptr), order_mng(nullptr), personnel_mng(nullptr)
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
			else return personnel_mng->fetch_task(pers_id, pt);
		}
		// Finish the task after the user performed
		void finish_task(const std::string& pers_id, Task* pt);

		void refund(const Refund_Order&);		

		// 生成任务，可以指定人员或时间执行
		struct Stock_Record
		{
			// Record a task a aux infos(maybe)
			Inventory_Task inventory_task;

			// Ctor
			Stock_Record(const Inventory_Task& itask) :
				inventory_task(itask) {}
			Stock_Record(const Stock_Record& oth) :
				inventory_task(oth.inventory_task) {}
			Stock_Record& operator =(const Stock_Record& rhs)
			{
				if (this != &rhs)
				{
					inventory_task = rhs.inventory_task;
				}
				return *this;
			}			
		};
		
		template<typename Container>
		bool check_stock(const Container& locs)
		{
			auto pers_id = personnel_mng->next_avail_personnle();
			if (pers_id.empty()) { return false; }
			return check_stock(locs, pers_id);
		}

		template<typename Container>
		bool check_stock(const Container& locs, const std::string& pers_id)
		{
			// Verify location type
			static_assert
			(
				std::is_same<Container::value_type, Location>::value,
				"Container must store Locations!"
			);
			
			// Generate task
			auto invtask = std::make_unique<Inventory_Task>();
			auto vii = std::vector<Inventory_Info>{};
			for (const auto& loc : locs)
			{
				// Verify location
				if (!item_mng->verify_location(loc)) { return false; }

				// Get the location's item info
				auto found = item_mng->check_location(loc);
				if (!found.first) { return false; }
				auto& item = found.second;
				vii.emplace_back(Inventory_Info{ item.item_id, loc, item.stocks });
			}

			// assign
			auto invtask = std::make_unique<Inventory_Task>(vii);
			return personnel_mng->assign(std::move(invtask), pers_id);
		}

	public:
		database* pdb;
		std::unique_ptr<order_manager>		order_mng;
		std::unique_ptr<item_manager>		item_mng;
		std::unique_ptr<personnel_manager>	personnel_mng;		
		std::vector<Stock_Record>			srec_vec;
		void finish_picking_task(Picking_Task* task);
		// Generate inventory report
		void finish_inventory_task(Inventory_Task* task)
		{
			srec_vec.emplace_back(Stock_Record{ *task });
		}
	};
}
#endif