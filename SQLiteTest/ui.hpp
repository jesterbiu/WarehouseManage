#pragma once
#include <iostream>
#include "warehouse.hpp"

namespace WarehouseManage
{
	class ui
	{
	private:
		
	public:
		warehouse* pw;
		std::string id;
		ui(warehouse* pw, const std::string& id) :
			pw(pw), id(id) {}
	
		/*
		void print_task(const Task*);
		const Task* pick_task();
		void perform_task(Task* pt)
		{
			switch (pt->tell_task_type())
			{
			case Task_Type::Picking_Type:
				auto p = dynamic_cast<Picking_Task*>(pt);
				perform_picking_task(p);
				break;
			default:
				break;
			}
		}
		// perform_task() helper functions
		void perform_picking_task(Picking_Task*);
		void perform_inventory_task(Inventory_Task*);
		*/
	};
	
}