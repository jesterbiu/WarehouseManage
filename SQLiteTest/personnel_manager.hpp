#pragma once
#include <unordered_map>
#include <queue>
#include "manager.hpp"
#include "personnel.hpp"
#include "task.hpp"
namespace WarehouseManage
{
	using Task_Queue = std::deque<std::unique_ptr<Task>>;
	using PersID_Task_Map = std::unordered_map<std::string, std::deque<std::unique_ptr<Task>>>;

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
			tasker = std::make_shared<task_dispatcher>();
		}
		// Copy ctor
		personnel_manager(const personnel_manager& oth) = delete;
		// Assignment operator
		personnel_manager& operator =(const personnel_manager& oth) = delete;		
		~personnel_manager() {}
	

		// Add a new personnel into database
		bool add_personnel(const Personnel&);
		// Find a personnel's info by id
		std::pair<bool, Personnel> find_personnel(const std::string& persid);	
		// Return the personnel's task queue
		inline const Task_Queue* fetch_task_queue(const std::string& pers_id)
		{
			return tasker->fetch_task_queue(pers_id);
		}
		inline Task* fetch_task(const std::string& pers_id, const Task* pt)
		{
			return tasker->fetch_task(pers_id, pt);
		}
		inline void finish_task(const std::string& pers_id, Task* pt)
		{
			tasker->finish_task(pers_id, pt);
		}
		// Return a list of all registered personnel's info
		std::vector<Personnel> personnel_list();
		// Return the next available personnel
		inline std::string next_avail_personnle()
		{
			return tasker->next_avail_personnel();
		}
		// Assign a job automatically using job_dispatch()
		inline std::pair<bool, std::string> assign(std::unique_ptr<Task>&& upt) 
		{
			return tasker->assign(std::move(upt));
		}
		// Assign a job to a specific personnel
		inline bool assign(std::unique_ptr<Task>&& upt, const std::string& pers_id)
		{
			if (find_personnel(pers_id).first)
			{
				return tasker->assign(std::move(upt), pers_id);
			}
			return false;
		}

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
		class task_dispatcher
		{			
			// <Personnel - task queue>
			PersID_Task_Map tasks;
			// Holds proceeding tasks for manager to inspect progress
			std::unordered_map<Task*, Task_Queue::iterator> proceeding_tasks;
			
		public:
			task_dispatcher() {}
			task_dispatcher(const std::vector<std::string>& plist)
			{
				for (auto& p : plist)
				{
					add_personnel(p);
				}
			}
			task_dispatcher(const task_dispatcher& oth) = delete;
			task_dispatcher& operator =(const task_dispatcher& rhs) = delete;

			// Add personnel to dispose given id
			bool add_personnel(const std::string&);
			// Return next available personnel. If there is none return empty string
			std::string next_avail_personnel()
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
			// Assign a task automatically
			std::pair<bool, std::string> assign(std::unique_ptr<Task>&&);
			// Assign a task to a designated personnel
			bool assign(std::unique_ptr<Task>&&, const std::string&);
			// Fetch and return the task queue given personnel id
			inline Task_Queue* fetch_task_queue(const std::string& pers_id)
			{
				if (tasks.find(pers_id) == tasks.end())
				{
					return nullptr;
				}
				else
				{
					return &tasks[pers_id];
				}				
			}
			// Activate a task to perform
			Task* fetch_task(const std::string& pers_id, const Task* pt);
			// Finish and delete a task from its task queue
			void finish_task(const std::string& pers_id, Task* pt);
		};
		std::shared_ptr<task_dispatcher> tasker;
				
		// find_personnel()'s helper function
		void extract_personnel(sqlite3_stmt*, Personnel&);
	};
}