#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <type_traits>
#include "item.hpp"
#define JOB_ 1
#if JOB_
namespace WarehouseManage
{
	// Public interface for jobs
	struct Task
	{
		virtual ~Task() {}
		virtual void perform() = 0;
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

	struct Pikcing_Job : public Task
	{
		Pikcing_Job(const Picking_Info& pinfo) : picking_info(pinfo) {}
		void perform() override;
		~Pikcing_Job() {}
	private:
		Picking_Info picking_info;
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
	struct Inventory_Job : public Task
	{
		Inventory_Job(const Inventory_Info& iinfo) : inventory_info(iinfo) {}
		void perform() override;
		~Inventory_Job() {}
	private:
		Inventory_Info inventory_info;
	};

	// Factory
	template<typename JobType, typename Arg>
	std::unique_ptr<Task> generate_job(Arg& arg)
	{
		static_assert(std::is_base_of<Task, JobType>::value, "type parameter of this class must derive from job");
		return std::make_unique<JobType>(arg);
	}
}

#endif