#pragma once
#include "database.hpp"

namespace WarehouseManage
{
	class manager
	{
	protected:
		// No default ctor requiring user 
		// to explicitly initialize the database instance
		manager() = delete;
		// Param ctor
		manager(database* pdb) : db(pdb) {}
		// Copy ctor
		manager(const manager& oth) : db(oth.db) {}
		// Assignment operator
		manager& operator =(const manager& oth)
		{
			if (this != &oth)
			{
				db = oth.db;
			}
			return *this;
		}
		virtual ~manager() {}

		inline sqlite3* get_database() const
		{ 
			return **db; 
		}
		
		// Verify that the step returns result not empty, else throw an exception
		void step_has_result(int rc, const std::string& func_name);

		// Includes functions generate statement_handle
		struct statement_generator 
		{
			static statement_handle generate_stmt_handle(const char* sqlstmt, sqlite3* db);
		};
	
	private:
		database* db;		
	};
}