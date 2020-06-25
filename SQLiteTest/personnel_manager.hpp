#pragma once
#include <unordered_map>
#include <queue>
#include "manager.hpp"
#include "personnel.hpp"
#include "task.hpp"
namespace WarehouseManage
{
	using Task_Queue = std::queue<Task>*;
	class personnel_manager : public manager
	{
	public:
		// No default ctor requiring user 
		// to explicitly initialize the database instance
		personnel_manager() = delete;
		// Param ctor
		personnel_manager(database* pdb) : 
			manager(pdb)
		{
			tasker = std::make_unique<task_dispatcher>();
		}
		// Copy ctor
		personnel_manager(const personnel_manager& oth) : 
			manager(oth) 
		{
			tasker = std::make_unique<task_dispatcher>(oth.tasker);
		}
		// Assignment operator
		personnel_manager& operator =(const personnel_manager& oth)
		{
			if (this != &oth)
			{
				manager::operator=(oth);
				tasker = std::make_unique<task_dispatcher>(oth.tasker);
			}
			return *this;
		}
		~personnel_manager() {}
	
		// Add a new personnel into database
		bool add_personnel(const Personnel&);
		
		// Find a personnel's info by id
		std::pair<bool, Personnel> find_personnel(const std::string& persid);
		
		// 
		std::vector<Personnel> personnel_list();

		// Assign a job automatically using job_dispatch()
		bool assign(std::unique_ptr<Task>&&);

		// Assign a job to a specific personnel
		bool assign(std::unique_ptr<Task>&&, const std::string& pers_id);

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

		class task_dispatcher
		{
			using PersID_Task_Map = std::unordered_map<std::string, std::queue<std::unique_ptr<Task>>>;
			PersID_Task_Map tasks;
			std::string next_personnel()
			{
				static auto iter = tasks.begin();

				// Return empty if no personnel is available 
				if (tasks.empty())
				{
					return std::string{};
				}
				
				// Assign tasks equally
				if (iter == tasks.end())
				{
					iter = tasks.begin();
				}
				auto& next_pers = iter->first;
				iter++;
				return next_pers;

			}
		public:
			task_dispatcher() {}
			task_dispatcher(const std::vector<std::string>& plist)
			{
				for (auto& p : plist)
				{
					add_personnel(p);
				}
			}
			task_dispatcher(const task_dispatcher& oth) :
				tasks(oth.tasks) {}
			task_dispatcher& operator =(const task_dispatcher& rhs)
			{
				if (this != &rhs)
				{
					tasks = rhs.tasks;
				}
				return *this;
			}

			bool add_personnel(const std::string&);
			bool assign(std::unique_ptr<Task>&&);
			bool assign(std::unique_ptr<Task>&&, const std::string&);
			const Task_Queue check_task(const std::string&);
		};
		std::unique_ptr<task_dispatcher> tasker;
	};
}