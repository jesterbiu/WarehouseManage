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
