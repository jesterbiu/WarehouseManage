#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include "item.hpp"
#define TASK_ 1
#if TASK_
namespace WarehouseManage
{
	enum class Task_Type { Picking_Type, Inventory_Type, Refunding_Type };

	// Public interface for jobs
	struct Task
	{
		virtual Task_Type tell_task_type() = 0;
		virtual ~Task() {}
	};

	// Item task: store or extract a certain quantity of items from the warehouse
	// given each item's stored location
	struct Item_Info
	{
		// Fields
		std::string		item_id;
		int				quantity;
		Location		location;

		// Ctor
		// item_id, quantity, location
		Item_Info(const std::string& iid, int qty, const Location& loc) :
			item_id(iid), quantity(qty), location(loc)
		{}
		Item_Info(const Item_Info& oth) :
			item_id(oth.item_id),
			quantity(oth.quantity),
			location(oth.location)
		{}
		Item_Info& operator =(const Item_Info& oth)
		{
			if (this != &oth)
			{
				item_id = oth.item_id;
				quantity = oth.quantity;
				location = oth.location;
			}
			return *this;
		}

		// Operator ==
		bool operator ==(const Item_Info& oth) const
		{
			return item_id == oth.item_id
				&& quantity == oth.quantity
				&& location == oth.location;
		}

	};	
	enum class Item_Type { ToBeShipped, NewArrival, Refunded };
	struct Item_Task : public Task
	{
		// Fields
		std::string order_id;
		std::vector<Item_Info> picking_infos;
		Item_Type item_type;

		// Ctor and dtor
		template<typename Container>
		Item_Task(const std::string& oid, const Container& infos, Item_Type itype) :
			order_id(oid),
			picking_infos(infos.begin(), infos.end()),
			item_type(itype)
		{}
		Item_Task(const Item_Task& oth) :
			order_id(oth.order_id),
			picking_infos(oth.picking_infos),
			item_type(oth.item_type)
		{}
		Item_Task(Item_Task&& oth) noexcept:
			order_id(std::move(oth.order_id)),
			picking_infos(std::move(oth.picking_infos)),
			item_type(oth.item_type)
		{}
		Item_Task& operator =(const Item_Task& rhs)
		{
			if (this != &rhs)
			{
				order_id = rhs.order_id;
				picking_infos = rhs.picking_infos;
				item_type = rhs.item_type;
			}
			return *this;
		}
		Item_Task& operator =(Item_Task&& rhs) noexcept
		{
			order_id = std::move(rhs.order_id);
			picking_infos = std::move(rhs.picking_infos);
			item_type = rhs.item_type;
			return *this;
		}
		~Item_Task() {}
		
		Task_Type tell_task_type() override
		{
			return Task_Type::Picking_Type; 
		}

		// Operator ==
		bool operator ==(const Item_Task& oth) const
		{
			return order_id == oth.order_id
				&& picking_infos == oth.picking_infos;
		}
	};

	// Inventory_Task: do the inventory
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
			expected_stock_count(expect), actual_stock_count(-1)
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
		inline bool operator <(const Inventory_Info& oth) const
		{
			return location < oth.location;
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
		Inventory_Task& operator =(Inventory_Task&& rhs) noexcept
		{
			inventory_infos = std::move(rhs.inventory_infos);
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