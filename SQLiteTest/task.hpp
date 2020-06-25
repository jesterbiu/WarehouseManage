#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include "warehouse.hpp"
#define TASK_ 1
#if TASK_
namespace WarehouseManage
{
	// Public interface for jobs
	struct Task
	{
		virtual ~Task() {}
	protected:
		warehouse* pwh;
	};

	// Picking_Job: pick a part of an order
	struct Picking_Info
	{
		// item_id, quantity, location
		Picking_Info(const std::string& iid, int qty, const Location& loc) :
			item_id(iid), quantity(qty), location(loc)
		{}
		std::string		item_id;
		int				quantity;
		Location		location;
	};
	struct Pikcing_Task : public Task
	{
		template<typename Container>
		Pikcing_Task(const Container& infos) :
			picking_infos(infos.begin(), infos.end())
		{}
		~Pikcing_Task() {}
		std::vector<Picking_Info> picking_infos;
	};

	// Inventory_Job: do the inventory
	struct Inventory_Info
	{
		// item_id, location, expected count, actual count
		Inventory_Info(const std::string& iid, const Location loc, int expect, int actual) :
			item_id(iid), location(loc),
			expected_stock_count(expect), actual_stock_count(actual)
		{}
		std::string item_id;
		Location location;
		int expected_stock_count;
		int actual_stock_count;
	};
	struct Inventory_Task : public Task
	{
		template<typename Container>
		Inventory_Task(const Container& infos) :
			inventory_infos(infos.begin(), infos.end())
		{}
		~Inventory_Task() {}
		std::vector<Inventory_Info> inventory_infos;
	};

	// Factory
	template<typename TaskType, typename Arg>
	std::unique_ptr<Task> generate_task(Arg& arg)
	{
		// Restrict that the task type must derive from Task
		static_assert
		(
			std::is_base_of<Task, TaskType>::value, 
			"task type must derive from Task"
		);
		return std::make_unique<TaskType>(arg);
	}
}
#endif