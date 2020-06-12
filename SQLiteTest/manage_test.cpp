#include "manage_test.hpp"
namespace Tests
{
	
		void manage_test()
		{
			
            std::vector<Item> vi;            
            try
            {
                // read file                
                storage::read_item_table("item_tab_1.csv", vi, Item_File_Type::Located);
                
                // initialize a manage instance
                auto db = database::get_instance();
                auto mp = manage(db);                

                // insert items to database 
                for (auto& i : vi)
                {
                    // add_item
                    if (!mp.add_item(i))
                    {
                        std::cout << "failed to ADD " << i.item_id << "\n";
                    }

                    // exist
                    if(!mp.exist(i.item_id))
                    {
                        std::cout << "failed to check " << i.item_id << " EXIST\n";
                    }

                    // find_item
                    auto found = mp.find_item(i.item_id);
                    if (! (found.first && found.second == i))
                    {
                        std::cout << "failed to FIND " << i.item_id << "\n";
                    }

                    // check_location
                    auto checked = mp.check_location(i.location);
                    if (! (checked.first && checked.second == i))
                    {
                        std::cout << "failed to LOCATE " << i.item_id << "\n";
                    }
                    
                    // Output success
                    std::cout << "item " << i.item_id << "\n";
                }

                // Test update                
                int diff = 3;
                for (auto& i : vi)
                {
                    auto updated_stocks = i.stocks + diff;
                    mp.update_stocks(i.item_id, updated_stocks);
                    auto r = mp.find_item(i.item_id);
                    if (! (r.first && r.second.stocks == updated_stocks))
                    {
                        std::cout << "failed to UPDATE " << i.item_id << "\n";
                    }             
                    std::cout << "item " << i.item_id << "(updated)\n";
                }
            }
            catch (warehouse_except& expt)
            {
                std::cout << expt.what();                
            }
		}
}