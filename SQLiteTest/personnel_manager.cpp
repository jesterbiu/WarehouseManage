#include "personnel_manager.hpp"
using namespace WarehouseManage;

// add_personnel()
bool personnel_manager::add_personnel(const Personnel& pers)
{
	// Validate input
	if (!pers)
	{
		return false;
	}

	// Prepare SQL
	auto stmthandle = personnel_statement_generator::add_pers_stmt(get_database());

	// Bind:
	// Bind role
	database::bind_int(*stmthandle, 1, role_to_int(pers.role));

	// Bind id
	database::bind_text(*stmthandle, 2, pers.personnel_id);

	// Bind password
	database::bind_text(*stmthandle, 3, pers.password);

	// Execute
	database::step(*stmthandle);
	tasker->add_personnel(pers.personnel_id);
	return true;
}

// find_personnel() and its helper function
void personnel_manager::extract_personnel(sqlite3_stmt* stmt, Personnel& pers)
{
	// Extract role
	auto ri = database::extract_int(stmt, 0);
	pers.role = int_to_role(ri);
	// Extract pers_id
	pers.personnel_id = database::extract_text(stmt, 1);
	// Extract password
	pers.password = database::extract_text(stmt, 2);
}
std::pair<bool, Personnel> personnel_manager::find_personnel(const std::string& persid)
{
	//Validate input
	if (persid.empty())
	{
		return std::make_pair(false, Personnel{});
	}

	// Prepare SQL
	auto stmthandle = personnel_statement_generator::find_pers_stmt(get_database());

	// Bind SQL
	database::bind_text(*stmthandle, 1, persid);

	// Execute
	auto rc = database::step(*stmthandle);
	step_has_result(rc, "personnel_manager::find_personnel()");

	// Extract
	auto pers = Personnel{};
	extract_personnel(*stmthandle, pers);

	return std::make_pair(true, pers);
}

bool personnel_manager::task_dispatcher::add_personnel(const std::string& pers_id)
{
	auto rt = tasks.try_emplace(pers_id, std::deque<std::unique_ptr<Task>>{});
	return rt.second;
}
std::pair<bool, std::string> personnel_manager::task_dispatcher::assign(std::unique_ptr<Task>&& task)
{
	// Select a personnel to assign to
	auto p = next_personnel();
	if (p.empty()) { return std::make_pair(false, std::string{}); }

	tasks[p].push_back(std::move(task));
	return std::make_pair(true, p);
}
bool personnel_manager::task_dispatcher::assign(std::unique_ptr<Task>&& task, const std::string& pers_id)
{
	// Check if the personnel is registered before assignment
	if (tasks.find(pers_id) == tasks.end())
	{
		return false;
	}
	tasks[pers_id].push_back(std::move(task));
	return true;
}
Task* personnel_manager::task_dispatcher::fetch_task(const std::string& pers_id, const Task* pt)
{
	if (tasks.find(pers_id) == tasks.end())
	{
		return nullptr;
	}
	// Find the task
	for (auto beg = tasks[pers_id].begin(); beg != tasks[pers_id].end(); beg++)
	{
		if (beg->get() == pt)
		{
			// Record the task being selected
			proceeding_tasks.emplace(beg->get(), beg);
			return beg->get();
		}
	}
	return nullptr;
}
void personnel_manager::task_dispatcher::finish_task(const std::string& pers_id, Task* pt)
{
	if (proceeding_tasks.find(pt) != proceeding_tasks.end())
	{
		auto it = proceeding_tasks[pt];

		// Delete the task from proceeding task list
		proceeding_tasks.erase(pt);

		// Delete the task from personnel's task queue
		tasks[pers_id].erase(it);
	}
}
