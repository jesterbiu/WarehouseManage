#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include "item.hpp"
#define TASK_ 1
#if TASK_
namespace WarehouseManage
{
	enum class Task_Type { Picking_Type, Inventory_Type };

	// Public interface for jobs
	struct Task
	{
		virtual Task_Type tell_task_type() = 0;
		virtual ~Task() {}
	};

	// Picking_Job: pick a part of an order
	struct Picking_Info
	{
		// Fields
		std::string		item_id;
		int				quantity;
		Location		location;

		// Ctor
		// item_id, quantity, location
		Picking_Info(const std::string& iid, int qty, const Location& loc) :
			item_id(iid), quantity(qty), location(loc)
		{}		
		Picking_Info(const Picking_Info& oth) :
			item_id(oth.item_id),
			quantity(oth.quantity),
			location(oth.location)
		{}
		Picking_Info& operator =(const Picking_Info& oth)
		{
			if (this != &oth)
			{
				item_id = oth.item_id;
				quantity = oth.quantity;
				location =oth.location;
			}
			return *this;
		}
			
		// Operator ==
		bool operator ==(const Picking_Info& oth) const
		{
			return item_id == oth.item_id
				&& quantity == oth.quantity
				&& location == oth.location;
		}

	};
	struct Picking_Task : public Task
	{
		// Fields
		std::string order_id;
		std::vector<Picking_Info> picking_infos;

		// Ctor and dtor
		template<typename Container>
		Picking_Task(const std::string& oid, const Container& infos) :
			order_id(oid),
			picking_infos(infos.begin(), infos.end())
		{}
		Picking_Task(const Picking_Task& oth) :
			order_id(oth.order_id),
			picking_infos(oth.picking_infos)
		{}
		Picking_Task(Picking_Task&& oth) noexcept:
			order_id(std::move(oth.order_id)),
			picking_infos(std::move(oth.picking_infos))
		{}
		Picking_Task& operator =(const Picking_Task& rhs)
		{
			if (this != &rhs)
			{
				order_id = rhs.order_id;
				picking_infos = rhs.picking_infos;
			}
			return *this;
		}
		~Picking_Task() {}
		
		Task_Type tell_task_type() override
		{
			return Task_Type::Picking_Type; 
		}

		// Operator ==
		bool operator ==(const Picking_Task& oth) const
		{
			return order_id == oth.order_id
				&& picking_infos == oth.picking_infos;
		}
	};

	// Inventory_Job: do the inventory
	struct Inventory_Info
	{
		// Fields
		std::string item_id;
		Location location;
		int expected_stock_count;
		int actual_stock_count;

		// item_id, location, expected count, actual count
		Inventory_Info(const std::string& iid, const Location loc, int expect) :
			item_id(iid), location(loc),
			expected_stock_count(expect), actual_stock_count(0)
		{}
		Inventory_Info(const Inventory_Info& oth) :
			item_id(oth.item_id), location(oth.location),
			expected_stock_count(oth.expected_stock_count), actual_stock_count(oth.actual_stock_count)
		{}
		Inventory_Info& operator = (const Inventory_Info& oth)
		{
			if (this != &oth)
			{
				item_id = oth.item_id;
				location = oth.location;
				expected_stock_count = oth.expected_stock_count;
				actual_stock_count = oth.actual_stock_count;
			}
			return *this;
		}
			
		
		// Operator ==
		bool operator ==(const Inventory_Info& oth) const
		{
			return item_id == oth.item_id
				&& location == oth.location;
		}
		
	};
	struct Inventory_Task : public Task
	{
		// Field
		std::vector<Inventory_Info> inventory_infos;

		// Ctor & dtor
		template<typename Container>
		Inventory_Task(const Container& infos) :
			inventory_infos(infos.begin(), infos.end())
		{}
		Inventory_Task(const Inventory_Task& oth) :
			inventory_infos(oth.inventory_infos)
		{}
		Inventory_Task(Inventory_Task&& oth) noexcept:
			inventory_infos(std::move(oth.inventory_infos))
		{}
		Inventory_Task& operator =(const Inventory_Task& rhs)
		{
			if (this != &rhs)
			{
				inventory_infos = rhs.inventory_infos;
			}
			return *this;
		}
		~Inventory_Task() {}
		
		Task_Type tell_task_type() 
		{ 
			return Task_Type::Inventory_Type; 
		}

		// Operator ==
		bool operator ==(const Inventory_Task& oth) const
		{
			return inventory_infos == oth.inventory_infos;
		}
	};
}
#endif