#include <unordered_map>
#include <queue>
#include <deque>
#include <ctime>
#include "manager.hpp"
#include "personnel.hpp"
namespace WarehouseManage
{
	

	class personnel_manager : public manager
	{
	public:
		// No default ctor requiring user 
		// to explicitly initialize the database instance
		personnel_manager() = delete;
		// Param ctor
		personnel_manager(database* pdb) : manager(pdb) {}
		// Copy ctor
		personnel_manager(const manager& oth) : manager(oth) {}
		// Assignment operator
		personnel_manager& operator =(const manager& oth)
		{
			if (this != &oth)
			{
				manager::operator=(oth);
			}
			return *this;
		}
		~personnel_manager() {}
	
		bool add_personnel();
		std::pair<bool, Personnel> check_personnel();
		bool appoint();
		bool perform();

	private:
		struct personnel_statement_generator : public statement_generator
		{

		};
		//std::unordered_map<Personnel, std::queue<Job>> jobs;
		//std::priority_queue<std::string, std::deque<Personnel>> pq;
	};
}