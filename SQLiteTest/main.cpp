#include <iostream>
#include "manage.hpp"
#include "manage_test.hpp"
void print_item(const Item& i)
{
    if (auto l = i.location)
    {
        std::cout << i.location.shelf << i.location.slot << "\t";
    }
     std::cout  << i.item_id << "\t"
        << i.stocks << "\n";
}

int main()
{
#define READ_TEST_1 FALSE
#if READ_TEST_1
    std::vector<Item> vi;
    ///*
    try
    {
        storage::read_item_table("item_tab_1.csv", vi, Item_File_Type::Located);
        for (auto& i : vi)
        {
            print_item(i);
        }
    }
    catch (warehouse_except& expt)
    {
        std::cout << expt.what();
        return -1;
    }

    vi.clear();
    //*/

    try
    {
        storage::read_item_table("item_tab_2.csv", vi, Item_File_Type::NotLocated);
        for (auto& i : vi)
        {
            print_item(i);
        }
    }
    catch (warehouse_except& expt)
    {
        std::cout << expt.what();
        return -1;
    }
#endif

    Tests::manage_test();

    return 0;

}

