#pragma once
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
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
			init_finishers();
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

		// Generate and assign an in-stocking task
		// Return true and the id of the worker assigned to if succeeds
		// Return false and error message if fails
		template<typename Container>
		std::pair<bool, std::string> in_stock(const Container& items)
		{
			// Verify location type
			static_assert
				(
					std::is_same<Container::value_type, Item>::value,
					"Container must store Items!"
				);

			std::vector<Item_Info> in_stock_list{};
			for (auto& i : items)
			{
				// ID
				auto& item_id = i.item_id;

				// Location
				auto loc = Location{};
				// add new arrival item to the database then retrieve its location
				if (!item_mng->exist(item_id))
				{
					Item new_item = i;
					new_item.stocks = 0;
					if (!item_mng->add_item(new_item))
					{
						std::string errmsg = "in_stock(): failed to add new item!";
						return std::make_pair(false, errmsg);
					}								
				}
				loc = item_mng->get_item(item_id).second.location;

				// Quantity
				auto qty = i.stocks;

				in_stock_list.emplace_back(Item_Info{ item_id, qty, loc });
			}// end of for

			// Generate task and assign
			auto picktask = std::make_unique<Item_Task>
				(std::string{ "IN STOCK" }, in_stock_list, Item_Type::InStock);
			return personnel_mng->assign(std::move(picktask));
		}

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
			auto vii = std::move(get_inventory_infos(locs));
			if (vii.empty()) 
			{ 
				std::string errmsg = "invalid or empty Location!";
				return std::make_pair(false, errmsg);
			};

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
		
		// pick()'s helper functions£º
		// Verify ordered goods and update their stocks
		std::pair<bool, std::string> verify_ordered_goods(const Order& order);
		// Generate and return the picking list of the order
		std::vector<Item_Info> get_good_info_list(const std::vector<good>& goods);

		// refund()'s helper functions
		// Verify refunded goods
		std::pair<bool, std::string> verify_refunded_goods(const Order& order, const Refund_Order& rorder);

		// check_inventory()'s helper functions£º
		// Generate inventory_infos for the task
		template<typename Container>
		std::vector<Inventory_Info> get_inventory_infos(const Container& locs)
		{
			std::vector<Inventory_Info> vii{};
			for (const auto& loc : locs)
			{
				// Verify location: ignore if the location is not valid
				if (!item_mng->verify_location(loc)){ continue; }

				// Get the item stored at the location
				auto found = item_mng->check_location(loc);		
				if (found.first) 
				{
					auto& item = found.second;
					vii.emplace_back(Inventory_Info{ item.item_id, loc, item.stocks });
				}				
			} // end of for
			return vii;
		}
		
		std::map<Task_Type, std::function<void(Task*)>> task_finisher;
		std::map<Item_Type, std::function<void(Item_Task*)>> item_task_finisher;
		void init_finishers();
		void finish_itemio_task(Task* task);
		void finish_instock_task(Item_Task* task)
		{
			add_stocks(task->item_infos);
		}
		void finish_picking_task(Item_Task* task);
		void finish_refund_task(Item_Task* task)
		{
			add_stocks(task->item_infos);
		}
		void add_stocks(const std::vector<Item_Info>& infos);
		// Generate and store a stock_record
		inline void finish_inventory_task(Task* task)
		{
			auto pi = dynamic_cast<Inventory_Task*>(task);
			srec_vec.emplace_back(Stock_Record{ *pi });
		}
	};
}
#endif