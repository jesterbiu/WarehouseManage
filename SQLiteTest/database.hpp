#pragma once
#include <tuple>
#include <iostream>
#include "sqlite3.h"
#include "warehouse_exception.hpp"



// warehouse database
namespace warehouse {
	class database
	{
	public:
		static database* get_instance()
		{
			static database db;
			return &db;
		}

		// Check if the database is open
		explicit operator bool() const
		{
			return db;
		}

		// Get the managed sqlite3*
		sqlite3* operator *()
		{
			return db;
		}

	private:
		// Default constructor 
		// Opens the database and create tables
		database();
		// Destructor
		~database() { if (db) sqlite3_close(db); }

		// Delete copy functions
		database(const database&) = delete;
		database& operator =(const database&) = delete;

		// Field
		sqlite3* db;

		// table creators
		void create_table(const char* sqlstmt);
		void create_item_table();
		void create_order_table();
		void create_refund_order_table();
	};

	struct statement_handle
	{
		// Constructor
		statement_handle() :
			stmt(nullptr)
		{}
		statement_handle(const char* sqlstmt, sqlite3* db) :
			stmt(nullptr)
		{
			auto rc = sqlite3_prepare_v2(
				db,
				sqlstmt,
				-1,						// read to the first '\0'
				&stmt,
				nullptr					// pointer to the past-end compiled pos
				);

			// Error handling
			if (rc != SQLITE_OK)
			{
				auto errmsg = sqlite3_errmsg(db);
				throw warehouse_exception{ errmsg };
			}
		}

		// Movable but not copiable
		statement_handle(statement_handle&& oth) noexcept :
			stmt(std::exchange(oth.stmt, nullptr))
		{ }
		statement_handle(const statement_handle& oth) = delete;
		statement_handle& operator =(statement_handle&& oth) noexcept
		{
			if (this != &oth)
			{
				stmt = std::exchange(oth.stmt, nullptr);
			}
			return *this;
		}
		statement_handle& operator =(const statement_handle& oth) = delete;
		~statement_handle()
		{
			if (stmt)
			{
				sqlite3_finalize(stmt);
			}
		}

		// Member functions
		sqlite3_stmt* operator *() { return stmt; }
		explicit operator bool() const
		{
			return stmt;
		}

		// Fields
		sqlite3_stmt* stmt;
	};
}