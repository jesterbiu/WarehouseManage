#include "manager.hpp"
using namespace WarehouseManage;

void manager::step_has_result(int rc, const std::string& func_name)
{
	static std::string error_msg = ": query return empty!";
	error_msg = func_name + error_msg;
	if (!database::step_has_result(rc))
	{
		throw warehouse_exception
		{
			error_msg.c_str()
		};
	}
}

statement_handle manager::statement_generator::generate_stmt_handle(const char* sqlstmt, sqlite3* db)
{
	auto stmth = statement_handle{ sqlstmt, db };
	database::verify_stmt_handle(stmth);
	return stmth;
}