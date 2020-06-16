#include "manage_test.hpp"
using namespace warehouse;
// Debug options
#define ADD_ITEM 1
#define EXIST 1
#define FIND_ITEM 1
#define CHECK_LOCATION 1
#define UPDATE_STOCKS 1

namespace Tests
{
	
		void manage_test()
		{
			
            std::vector<Item> vi;            
            try
            {
                // read file                
                storage::read_item_table(".\\Data\\item_tab_1.csv", vi, Item_File_Type::Located);                

                // initialize a item_manager instance
                auto db = database::get_instance();
                auto mp = item_manager(db);                

                // insert items to database 
                int iter = 0;
                for (auto& i : vi)
                {
#if ADD_ITEM
                    // add_item
                    if (!mp.add_item(i))
                    {
                        std::cout << "failed to ADD " << i.item_id << "\n";
                    }
#endif
#if EXIST
                    // exist
                    if(!mp.exist(i.item_id))
                    {
                        std::cout << "failed to check " << i.item_id << " EXIST\n";
                    }
#endif
#if FIND_ITEM
                    // find_item
                    auto found = mp.find_item(i.item_id);
                    if (! (found.first && found.second == i))
                    {
                        std::cout << "failed to FIND " << i.item_id << "\n";
                    }
#endif
#if CHECK_LOCATION
                    // check_location
                    auto checked = mp.check_location(i.location);
                    if (! (checked.first && checked.second == i))
                    {
                        std::cout << "failed to LOCATE " << i.item_id << "\n";
                    }
#endif
                    // Output success
                    ++iter;
                    std::cout << "item " << iter << " " 
                        << i.item_id << "\n";                   
                }

#if UPDATE_STOCKS
                // Test update                
                int diff = 3;
                iter = 0;
                for (auto& i : vi)
                {
                    auto updated_stocks = i.stocks + diff;
                    mp.update_stocks(i.item_id, updated_stocks);
                    auto r = mp.find_item(i.item_id);
                    if (! (r.first && r.second.stocks == updated_stocks))
                    {
                        std::cout << "failed to UPDATE " << i.item_id << "\n";
                    }             
                    ++iter;
                    std::cout << "item " << iter << " " 
                        << i.item_id << "(updated)\n";

                }// end of for
#endif

                test_part_two(mp);
            }
            catch (warehouse_except& expt)
            {
                std::cout << expt.what();                
            }
		}

        void test_part_two(item_manager& mp)
        {
            std::vector<Item> vi;
           
            // read file                
            storage::read_item_table(".\\Data\\item_tab_2.csv", vi, Item_File_Type::NotLocated);

            // Add
            auto iter = 0;
            for (auto& i : vi)
            {
                if (!mp.add_item(i))
                {
                    std::cout << "failed to ADD " << i.item_id << "\n";
                }

                // exist
                if (!mp.exist(i.item_id))
                {
                    std::cout << "failed to check " << i.item_id << " EXIST\n";
                }

                // find_item
                auto found = mp.find_item(i.item_id);
                if (!found.first 
                    || i.item_id != found.second.item_id 
                    || i.stocks != found.second.stocks)
                {
                    std::cout << "failed to FIND " << i.item_id << "\n";
                }
                else
                {
                    i.location = found.second.location;
                }

                // check_location
                auto checked = mp.check_location(i.location);
                if (!(checked.first && checked.second == i))
                {
                    std::cout << "failed to LOCATE " << i.item_id << "\n";
                }

                ++iter;
                std::cout << "item " << iter << " "
                    << i.item_id << "\n";
            }
        }
}