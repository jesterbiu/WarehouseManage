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
	private:
		database* pdb;
		std::unique_ptr<order_manager>		order_mng;
		std::unique_ptr<item_manager>		item_mng;
		std::unique_ptr<personnel_manager>	personnel_mng;		
	public:
		// Ctor
		warehouse() :
			pdb(database::get_instance()),
			order_mng(nullptr),
			item_mng(nullptr), 
			personnel_mng(nullptr)
		{
			// Validate database connection first
			if (!pdb)
			{
				throw warehouse_exception
				{
					"warehouse(): failed to get database instance!"
				};
			}			
			
			// Initialize managers after
			item_mng = std::make_unique<item_manager>(pdb);
			order_mng = std::make_unique<order_manager>(pdb);
			personnel_mng = std::make_unique<personnel_manager>(pdb);

			// Initialize task-finishers
			init_finishers();
		}
		
	public:	// MEDIATOR FUNCTIONS FOR HIGH LEVEL TASKS
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
		inline const std::vector<Stock_Record>& get_stock_records()
		{
			return stock_records;
		}

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

	public:	// FACADE FUNCTIONS FOR MANAGERS
		// item_manager APIs
		// Add a unique item to the database. Return true if success
		bool add_item(const Item& item) 
		{ 
			return item_mng->add_item(item); 
		}
		// Check if the item of given id exists. Return true if exists
		bool item_exist(const std::string& id)
		{
			return item_mng->exist(id);
		}
		// Find and return the item if exists. Test pair.first to validate the return
		std::pair<bool, Item> get_item(const std::string& id)
		{
			return item_mng->get_item(id);
		}
		// Return the total count of items
		int item_count()
		{
			return item_mng->item_count();
		}
		// Try to store all but max_count of items in the container which the iter belongs to
		template<typename InputIt>
		void get_all_items(InputIt iter, size_t max_count)
		{
			item_mng->get_all_items(iter, max_count);
		}
		// Check the location and return the item at the location if there is one.
		std::pair<bool, Item> check_item_location(const Location& location)
		{
			return item_mng->check_location(location);
		}
		// Update an item's stocks to currstock given its id
		bool update_item_stocks(const std::string& id, int updated_stock)
		{
			return item_mng->update_stocks(id, updated_stock);
		}
		// Return true if the given location is legal
		bool verify_item_location(const Location& location)
		{
			return item_mng->verify_location(location);
		}

		// order_manager APIs
		// Add a new order to database; return true on success
		bool add_order(const Order& o)
		{
			return order_mng->add_order(o);
		}
		// Add a new refund order to database; return true on success
		bool add_refund_order(const Refund_Order& ro)
		{
			return order_mng->add_refund_order(ro);
		}
		// Check if an order exists; return true if exists
		bool order_exist(const std::string& order_id)
		{
			return order_mng->exist(order_id);
		}
		// Check and return an order's status given order id
		std::pair<bool, Order_Status_Type> check_order_status(const std::string& order_id)
		{
			return order_mng->check_status(order_id);
		}
		// Update an order's status given order id
		bool update_order_status(const std::string& order_id, const Order_Status_Type nstatus)
		{
			return order_mng->update_status(order_id, nstatus);
		}
		// Return the total count of orders
		int order_count()
		{
			return order_mng->order_count();
		}
		// Try to store all but max_count of orders in the container 
		// which the iter belongs to
		template<typename StrInputIt>
		void get_all_order_ids(StrInputIt iter, size_t max_count) 
		{
			order_mng->get_all_order_ids(iter, max_count);
		}
		// Fetch all orders of the given status using container.insert()
		template<typename StrContainer>
		void get_order_ids_by_status(StrContainer& c, Order_Status_Type status)
		{
			order_mng->get_order_ids_by_status(c, status);
		}
		// Get order from database
		std::pair<bool, Order> get_order(const std::string& order_id)
		{
			return order_mng->get_order(order_id);
		}
		// Get refund order from database
		std::pair<bool, Refund_Order> get_refund_order(const std::string& order_id)
		{
			return order_mng->get_refund_order(order_id);
		}
	
		// personnel_manager APIs
		// Add a new personnel into database
		bool add_personnel(const Personnel& pers)
		{
			return personnel_mng->add_personnel(pers);
		}
		// Find a personnel's info by id
		std::pair<bool, Personnel> get_personnel(const std::string& persid)
		{
			return personnel_mng->find_personnel(persid);
		}		
		// Return a list of all registered personnel's info
		std::vector<Personnel> get_personnel_list()
		{
			return personnel_mng->personnel_list();
		}		
		
	private:		
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
		
		// map: task -> task_dealer
		std::map<Task_Type, std::function<void(Task*)>> task_finisher;
		std::map<Item_Type, std::function<void(Item_Task*)>> item_task_finisher;
		void init_finishers();

		// finish() and its helper functions
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
		std::vector<Stock_Record> stock_records;
		inline void finish_inventory_task(Task* task)
		{
			auto pi = dynamic_cast<Inventory_Task*>(task);
			stock_records.emplace_back(Stock_Record{ *pi });
		}
	};
}
#endif