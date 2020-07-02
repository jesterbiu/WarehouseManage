#pragma once
#include <memory>
#include <iostream>
#include <algorithm>
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
		
	public:			
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

		// Generate and assign an order-picking task
		// Return true and the id of the worker assigned to if succeeds
		// Return false and error message if fails
		std::pair<bool, std::string> pick(const Order&);

		// Generate and assign a refund task
		// Return true and the id of the worker assigned to if succeeds
		// Return false and error message if fails
		std::pair<bool, std::string> refund(const Refund_Order&);

		// Contains a Inventory_Task record
		struct Stock_Record
		{
			// Record a task a aux infos(maybe)
			Inventory_Task inventory_task;

			// Ctor
			Stock_Record(const Inventory_Task& itask) :
				inventory_task(itask) {}
			Stock_Record(const Stock_Record& oth) :
				inventory_task(oth.inventory_task) {}
			Stock_Record(Stock_Record&& oth) noexcept :
				inventory_task(std::move(oth.inventory_task)) {}
			Stock_Record& operator =(const Stock_Record& rhs)
			{
				if (this != &rhs)
				{
					inventory_task = rhs.inventory_task;
				}
				return *this;
			}
			Stock_Record& operator =(Stock_Record&& rhs) noexcept
			{
				inventory_task = std::move(rhs.inventory_task);
				return *this;
			}

			// Return map of item whose actual stock is differ from the expected
			std::map<Inventory_Info, int> get_differences();
		};
		
		// Generate and assign a inventory-checking task
		// Return true and the id of the worker assigned to if succeeds
		// Return false and empty string if fails
		template<typename Container>
		std::pair<bool, std::string> check_inventory(const Container& locs)
		{
			auto pers_id = personnel_mng->next_avail_personnle();
			if (pers_id.empty()) { return std::make_pair(false, std::string{}); }
			return check_inventory(locs, pers_id);
		}
		template<typename Container>
		std::pair<bool, std::string> check_inventory(const Container& locs, const std::string& pers_id)
		{
			// Verify location type
			static_assert
			(
				std::is_same<Container::value_type, Location>::value,
				"Container must store Locations!"
			);
					
			// Generate inventory_infos for the task
			std::vector<Inventory_Info> vii{};
			for (const auto& loc : locs)
			{
				// Verify location
				if (!item_mng->verify_location(loc)) 
				{ return std::make_pair(false, std::string{}); }

				// Get the location's item info
				auto found = item_mng->check_location(loc);
				// Continue if no item stored at the location
				if (!found.first) { continue; }
				auto& item = found.second;
				vii.emplace_back(Inventory_Info{ item.item_id, loc, item.stocks });
			}

			// Sort locations in ascending order
			auto inventory_info_cmp = [](const Inventory_Info& ia, const Inventory_Info& ib)
			{ 
				return ia.location < ib.location; 
			};
			std::sort(vii.begin(), vii.end(), inventory_info_cmp);

			// Assign
			auto invtask = std::make_unique<Inventory_Task>(vii);
			auto assign_result = personnel_mng->assign(std::move(invtask), pers_id);
			return std::make_pair(assign_result, pers_id);
		}

	public:// change to private later
		database* pdb;
		std::unique_ptr<order_manager>		order_mng;
		std::unique_ptr<item_manager>		item_mng;
		std::unique_ptr<personnel_manager>	personnel_mng;		
		std::vector<Stock_Record>			srec_vec;
		
		// Verify ordered goods and update their stocks
		std::pair<bool, std::string> verify_ordered_goods(const Order& order);
		// Generate and return the picking list of the order
		std::vector<Item_Info> get_good_info_list(const std::vector<good>& goods);

		// Verify refunded goods
		std::pair<bool, std::string> verify_refunded_goods(const Order& order, const Refund_Order& rorder);

		void finish_picking_task(Item_Task* task);
		// Generate and store a stock_record
		inline void finish_inventory_task(Inventory_Task* task)
		{
			srec_vec.emplace_back(Stock_Record{ *task });
		}
	};
}
#endif