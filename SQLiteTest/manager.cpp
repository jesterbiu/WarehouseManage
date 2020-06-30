#include "manager.hpp"
using namespace WarehouseManage;

statement_handle manager::statement_generator::generate_stmt_handle(const char* sqlstmt, sqlite3* db)
{
	auto stmth = statement_handle{ sqlstmt, db };
	database::verify_stmt_handle(stmth);
	return stmth;
}