#include "pers_test.hpp"
using namespace WarehouseManage;

// Debug options
#define ADD_PERSONNEL 1
#define FIND_PERSONNEL 1
namespace Tests
{
	void pers_test()
	{
		auto vp = std::vector<Personnel>{};
		try
		{
			storage::read_pers_table(".\\Data\\pers_tab_1.csv", vp);
			auto pdb = database::get_instance();
			auto pers_mngr = personnel_manager(pdb);

			for (auto& p : vp)
			{
				std::cout << p.personnel_id << ": ";

#if ADD_PERSONNEL
				if (pers_mngr.add_personnel(p))
				{
					std::cout << "ADDED ";
				}
				else return;
#endif
#if FIND_PERSONNEL
				auto found = pers_mngr.find_personnel(p.personnel_id);
				if (found.first && found.second == p)
				{
					std::cout << "FOUND ";
				}
#endif
				std::cout << "\n";
			}
		}
		catch (warehouse_exception& we)
		{
			std::cout << we.what() << std::endl;
		}
	}
}
