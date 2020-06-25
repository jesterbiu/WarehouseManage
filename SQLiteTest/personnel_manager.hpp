#include <unordered_map>
#include <queue>
#include "manager.hpp"
#include "personnel.hpp"
#include "task.hpp"
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
	
		// Add a new personnel into database
		bool add_personnel(const Personnel&);
		
		// Find a personnel's info by id
		std::pair<bool, Personnel> find_personnel(const std::string& persid);
		
		// Assign a job automatically using job_dispatch()
		//bool assign(const Job&);

		// Assign a job to a specific personnel
		//bool assign(const Job&, const std::string& persid);

	private:
		struct personnel_statement_generator : public statement_generator
		{
			inline static statement_handle add_pers_stmt(sqlite3* db)
			{
				static auto sqlstmt 
					= "INSERT INTO personnels(role, pers_id, password) VALUES($r, $pid, $pw)";
				return generate_stmt_handle(sqlstmt, db);
			}
			inline static statement_handle find_pers_stmt(sqlite3* db)
			{
				static auto sqlstmt
					= "SELECT * FROM personnels WHERE pers_id = $pid";
				return generate_stmt_handle(sqlstmt, db);
			}
		};

		void extract_personnel(sqlite3_stmt*, Personnel&);

		class job_dispatcher
		{
			//std::unordered_map<Personnel, std::queue<Job>> jobs;
			//std::priority_queue<std::string, std::deque<Personnel>> pq;
		};
		
	};
}