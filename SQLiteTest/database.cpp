#include "database.hpp"
// The 2nd argument to the sqlite3_exec() callback function is the number of columns in the result.
// The 3rd argument to the sqlite3_exec() callback is an array of pointers to strings obtained 
//		as if from sqlite3_column_text(), one for each column.
//		If an element of a result row is NULL then the corresponding string pointer for the sqlite3_exec() callback is a NULL pointer.
//The 4th argument to the sqlite3_exec() callback is an array of pointers to strings 
//		where each entry represents the name of corresponding result column 
//		as obtained from sqlite3_column_name().
int create_table_callback(void* outside, int row_count, char** col_vals, char** col_names)
{
	std::cout << "create table"
		<< "\nrow count: " << row_count << std::endl;
	if ((col_names) && (*col_names))
	{
		std::cout << col_names[0] << "\n";
	}
	else
	{
		std::cout << "ERROR: failed to extract column name(s)\n";
	}
	if ((col_vals) && (*col_vals))
	{
		std::cout << col_vals[0] << "\n";
	}
	else
	{
		std::cout << "ERROR: failed to extract column value(s)\n";
	}
	return 0;
}

database::database()
{
	// Open an instance
	auto rc = sqlite3_open(":memory:", &db);
	if (SQLITE_OK != rc)
	{
		std::cout << sqlite3_errmsg(db);
		db = nullptr;
		return;
	}

	// Create item table
	const char sqlstmt[] =
		"CREATE TABLE items (shelf INTEGER, slot INTEGER, id TEXT PRIMARY KEY NOT NULL, stocks INTEGER)";	
	char* errmsg = NULL;	
	rc = sqlite3_exec(db, sqlstmt, create_table_callback, 0, &errmsg);	
	if (rc != SQLITE_OK)
	{
		std::cout << "FAILED to prepare create table: ";
		if (errmsg)
		{
			std::cout << errmsg << "\n";
		}
		else
		{
			std::cout << "NULL PTR";
		}
		db = nullptr;
		return;
	}
	
	// Validate creation
	const char sql[] =
		"SELECT name FROM sqlite_temp_master WHERE type = 'table' AND name = 'items';";
	rc = sqlite3_exec(db, sql, create_table_callback, 0, &errmsg);
	
	if (rc != SQLITE_OK)
	{
		std::cout << "FAILED to find created table: ";
		if (errmsg)
		{
			std::cout << errmsg << "\n";
		}
		else
		{
			std::cout << "NULL PTR";
		}
		db = nullptr;
		return;
	}
//*/	
}