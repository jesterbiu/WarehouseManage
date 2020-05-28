#pragma once
#include "warehouse.hpp"
//����ģʽ��manager
//��������ģʽ���������֮���
//���ģʽ�����νṹ
//�ɴ������ģʽ�����л� & ���ݷ��ʶ���ģʽ
//ǰ�˿�����ģʽ��map<����������������>
//�۲���ģʽ
//����¼ģʽ


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