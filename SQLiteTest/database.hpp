#pragma once
#include <tuple>
#include <iostream>
#include "sqlite3.h"
// warehouse database
class database
{
public:
	static database* get_instance()
	{
		static database db;
		return &db;
	}

	// Check if the database is open
	explicit operator bool()
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
};

struct statement_handle
{
	// Constructor
	statement_handle() :
		stmt(nullptr)
	{}
	statement_handle(const char* sqlstmt, sqlite3* db)
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
			stmt = nullptr;
		}
	}
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
	explicit operator bool() { return stmt; }

	// Fields
	sqlite3_stmt* stmt;
};