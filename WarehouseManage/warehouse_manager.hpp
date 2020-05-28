#pragma once
#include "warehouse.hpp"
//单例模式：manager
//工厂方法模式：建造货架之类的
//组合模式：树形结构
//可传输对象模式：序列化 & 数据访问对象模式
//前端控制器模式：map<操作，操作处理器>
//观察者模式
//备忘录模式


#define COMPILE_MANAGER 0
#if COMPILE_MANAGER

class order
{
public:	
	unsigned order_id;
	std::vector<id_type> items;
	std::vector<unsigned> quantities;
};

class picking_list
{
public:
	order* order_;
	std::vector<location_type> locations;
};

class warehouse_manager
{
public:	
	// Return the counts and postion of the item given its id. 
	// The bool returned indicates if the query succeeds.
	std::tuple<bool, item_info> item_query(const id_type&);

	// Return the item's id and stock counts given its position.
	// The bool returned indicates if the position stores an item.
	std::tuple<bool, slot_info> slot_query(const location_type&);

	// Return true if the item is in stock
	bool in_stock(const id_type&);

	
};

#endif