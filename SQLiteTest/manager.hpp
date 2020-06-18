#pragma once
#include "database.hpp"

namespace warehouse
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
		
		// Return SQLITE_DONE if successfully INSERT/UPDATE
		// Return SQLITE_DONE or SQLITE_ROW if successfully SELECT
		// Throws an exception otherwise
		virtual int step(sqlite3_stmt* stmthandle)
		{
			auto rc = sqlite3_step(stmthandle);

			// Error
			if (rc != SQLITE_DONE && rc != SQLITE_ROW)
			{
				switch (rc)
				{
				case SQLITE_BUSY:
					throw warehouse_exception("SQLITE_BUSY", rc);
				case SQLITE_ERROR:
					throw warehouse_exception("SQLITE_ERROR", rc);
				case SQLITE_MISUSE:
					throw warehouse_exception("SQLITE_MISUSE", rc);
				default:
					throw warehouse_exception("OTH", rc);
				}
			}
			return rc;
		}
		
		// Includes functions generate statement_handle
		struct statement_generator {};
	
	private:
		database* db;
		
	};
}