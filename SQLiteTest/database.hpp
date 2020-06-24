#pragma once
#include <tuple>
#include <iostream>
#include "sqlite3.h"
#include "warehouse_exception.hpp"

// warehouse database
namespace WarehouseManage {

	using ustring = std::basic_string<unsigned char>;

	struct statement_handle
	{
		// Constructor
		statement_handle() :
			stmt(nullptr)
		{}
		statement_handle(const char* sqlstmt, sqlite3* db) :
			stmt(nullptr), sql_statement(sqlstmt)
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
			stmt(std::exchange(oth.stmt, nullptr)),
			sql_statement(oth.sql_statement)
		{ }
		statement_handle(const statement_handle& oth) = delete;
		statement_handle& operator =(statement_handle&& oth) noexcept
		{
			if (this != &oth)
			{
				stmt = std::exchange(oth.stmt, nullptr);
				sql_statement = oth.sql_statement;
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
		bool valid() const { return stmt; }

		// Fields
		std::string sql_statement;		
	private:
		// Fields
		sqlite3_stmt* stmt;
	};
	
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

		// Binder
		inline static int bind_text(sqlite3_stmt* stmt, int one_based_index, const std::string& text)
			{
				auto rc = sqlite3_bind_text(
					stmt,
					one_based_index,
					text.c_str(),
					text.size(),
					nullptr
					);
				verify_binding(rc);
				return rc;
			}
		inline static int bind_int(sqlite3_stmt* stmt, int one_based_index, int value)
			{
				auto rc = sqlite3_bind_int(stmt, one_based_index, value);
				verify_binding(rc);
				return rc;
			}
		
		// Extractor
		inline static std::string extract_text(sqlite3_stmt* stmt, int zero_based_index)
		{
			auto ustr = ustring{ sqlite3_column_text(stmt, zero_based_index) };
			return std::string{ ustr.cbegin(), ustr.cend() };
		}
		inline static int extract_int(sqlite3_stmt* stmt, int zero_based_index)
		{
			return sqlite3_column_int(stmt, zero_based_index);
		}
		
		// Step
		static int step(sqlite3_stmt* stmthandle)
		{
			auto rc = sqlite3_step(stmthandle);
			verify_steping(rc);
			return rc;
		}

		// Return code analysis
		inline static void verify_stmt_handle(const statement_handle & pstmt)
		{
			if (pstmt)
			{
				// valid!
			}
			else
			{
				throw warehouse_exception
				{
					"statement_handle initialization failed!"
				};
			}
		}
		inline static void verify_binding(int bind_rc)
		{
			if (SQLITE_OK != bind_rc)
			{
				throw warehouse_exception
				{
					"bind failed!"
				};
			}
		}
		inline static void verify_steping(int step_rc)
		{
			if (step_rc != SQLITE_DONE && step_rc != SQLITE_ROW)
			{
				switch (step_rc)
				{
				case SQLITE_BUSY:
					throw warehouse_exception("step: SQLITE_BUSY", step_rc);
				case SQLITE_ERROR:
					throw warehouse_exception("step: SQLITE_ERROR", step_rc);
				case SQLITE_MISUSE:
					throw warehouse_exception("step: SQLITE_MISUSE", step_rc);
				default:
					throw warehouse_exception("step: OTH", step_rc);
				}
			}
		}
		inline static bool step_has_result(int step_rc)
		{
			return step_rc == SQLITE_ROW;
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
		void create_personnel_table();
	};

	
}