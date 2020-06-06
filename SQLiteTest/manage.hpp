#pragma once
#include <iostream>
#include <tuple>
#include <memory>
#include <utility>
#include <cmath>
#include "error.hpp"
#include "sqlite3.h"
#include "item.hpp"

class manage
{
private:	
	class database
	{
	public:
		// Constructor
		database(const char* filename)
		{
			auto rc = sqlite3_open(filename, &db);
			if (SQLITE_OK != rc)
			{
				std::cout << sqlite3_errmsg(db);
				db = nullptr;
			}
		}
		database(const database&) = delete;
		database& operator =(const database&) = delete;

		// Destructor
		~database() { if(db) sqlite3_close(db); }

		explicit operator bool()
		{
			return db;
		}

		sqlite3* operator *()
		{
			return db;
		}

		// Print error
		bool is_error(int rc, char* errmsg = nullptr) 
		{
			if (rc != SQLITE_OK)
			{
				if (errmsg)
				{
					std::cout << errmsg << std::endl;
				}
				else
				{
					std::cout << "sqlite error" << std::endl;
				}

				return true;
			}
			return false;
		}

		// Exec
		/*
		bool query(
			const std::string& sqlstmt,
			int (*extract_result)(void*, int, char**, char**),
			void* obj
			)
		{			
			auto * sqlstmt_cstr = sqlstmt.c_str();			
			char* errmsg = nullptr;
			auto rc = sqlite3_exec(db, sqlstmt_cstr, extract_result, obj, &errmsg);
			if (is_error(rc, errmsg))
			{
				return false;
			}
			return true;
		}

		template <typename T>
		bool insert(
			const std::string& sqlstmt,
			const std::vector<T> datas

			) {
			return false;
		}
	*/
		
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
		statement_handle(statement_handle&& oth) :
			stmt(std::exchange(oth.stmt, nullptr))
		{ }
		statement_handle& operator =(const statement_handle&& oth)
		{
			if (this != &oth)
			{
				stmt = std::exchange(oth.stmt, nullptr);
			}
			return *this;
		}
		
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
public:
	// items (shelf INTEGER, slot INTEGER, id TEXT, stocks INTEGER)
	manage() :
		db(":memory:")
	{
		// Create table
		if (db)
		{
			const char sqlstmt[] = 
				"CREATE TABLE items (shelf INTEGER, slot INTEGER, id TEXT, stocks INTEGER)";
			sqlite3_stmt* stmthandle;
			auto rc = sqlite3_prepare_v2(db.db, sqlstmt, -1, &stmthandle, NULL);
			if (rc != SQLITE_OK)
			{
				std::cout << "FAILED to prepare create table: code " << rc << " \n";
				db.db = nullptr;
				return;
			}
			rc = sqlite3_step(stmthandle);
			if (rc != SQLITE_DONE && rc != SQLITE_ROW)
			{
				std::cout << "FAILED to step create table: code " << rc << " \n";
				db.db = nullptr;
				return;
			}
			sqlite3_finalize(stmthandle);
			std::cout << "create table\n";
		}
	}
		
	bool add_item(const Item& item)
	{
		// Prepare statement
		auto stmthandle = insert_stmt();
		if (!stmthandle)
		{
			return false;
		}

		// Bind				
		bind_insert(item, *stmthandle);
		
		// Execute
		return SQLITE_DONE == step(*stmthandle)
			? true
			: false;
	}

	bool exist(const std::string& id)
	{
		auto result = query_by_id(id);
		return result.first;
	}

	std::pair<bool, Item> find_item(const std::string& id)
	{
		// Check existence
		auto result = query_by_id(id);
		if (!result.first)
		{
			return std::make_pair(false, Item());
		}

		// Extract results
		auto d = Item{};
		extract_query(d, *result.second);	

		return std::make_pair(true, d);
	}
	
	std::pair<bool, Item> check_location(const Location& location)
	{
		// Prepare statement
		auto stmthandle = location_query_stmt();
		if (!stmthandle)
		{
			return std::make_pair(false, Item());
		}

		// Bind sql parameter to values
		sqlite3_bind_int(*stmthandle, 1, location.shelf);
		sqlite3_bind_int(*stmthandle, 2, location.slot);

		// Execute sql
		auto rc = step(*stmthandle);
		if (rc != SQLITE_DONE && rc != SQLITE_ROW)
		{
			return std::make_pair(false, Item());
		}
		
		// Extract results
		auto d = Item{};
		extract_query(d, *stmthandle);

		return std::make_pair(true, d);

	}

private:
	// Return a statement_handle which selects item by id
	statement_handle id_query_stmt()
	{
		static auto sqlstmt =
			"SELECT shelf, slot, id, stocks FROM items WHERE id = $id_";			
		return statement_handle{ sqlstmt, *db };
	}

	// Return a statement_handle which inserts an item
	statement_handle insert_stmt()
	{
		static auto sqlstmt =
			"INSERT INTO items(shelf, slot, id, stocks) VALUES($shelf, $slot, $id_, $stocks_)";
		return statement_handle{ sqlstmt, *db };
	}

	// Return a statement_handle which selects item by location
	statement_handle location_query_stmt()
	{
		static auto sqlstmt =
			"SELECT shelf, slot, id, stocks FROM items WHERE shelf = $shelf_ AND slot = $slot_";
		return statement_handle{ sqlstmt, *db };
	}

	// Bind data to write to (QUERY)
	void extract_query(Item& d, sqlite3_stmt* stmthandle)
	{
		d.location = Location
		{
			(char)sqlite3_column_int(stmthandle, 0),	// shelf id
			std::abs(sqlite3_column_int(stmthandle, 1)),					// slot index
		};
		d.item_id = (const char*)sqlite3_column_text(stmthandle, 2);
		d.stocks = std::abs(sqlite3_column_int(stmthandle, 3));

	}

	// Bind data to read from (INSERT, UPDATE)
	void bind_insert(const Item& d, sqlite3_stmt* stmthandle)
	{
		// Bind location
		sqlite3_bind_int(stmthandle, 0, d.location.shelf);
		sqlite3_bind_int(stmthandle, 1, d.location.slot);

		// Bind id
		const char* id = d.item_id.c_str();
		sqlite3_bind_text(
			stmthandle,
			2,
			id,
			d.item_id.size(),	// the offset of '\0'
			nullptr				// no deleter
			);

		// Bind stocks
		sqlite3_bind_int(stmthandle, 3, d.stocks);
	}
	
	// Fetch a row of data and print error if there is any
	int step(sqlite3_stmt* stmthandle)
	{
		auto rc = sqlite3_step(stmthandle);
		// Error
		if (rc != SQLITE_DONE)
		{
			switch (rc)
			{
			case SQLITE_BUSY:
				throw warehouse_except("SQLITE_BUSY", rc);
			case SQLITE_ERROR:
				throw warehouse_except("SQLITE_ERROR", rc);
			case SQLITE_MISUSE:
				throw warehouse_except("SQLITE_MISUSE", rc);
			default:
				throw warehouse_except("OTH", rc);
			}
		}
		return rc;
	}
	
	// Query by item id
	std::pair<bool, statement_handle> query_by_id(const std::string& id)
	{
		// Prepare statement
		auto stmthandle = id_query_stmt();
		if (!stmthandle)
		{
			return std::make_pair(false, stmthandle);
		}

		// Bind sql parameter to input value
		sqlite3_bind_text(
			*stmthandle, 1,
			id.c_str(),
			id.size(),	// offest of '\0'
			nullptr		// no deleter
			);

		// Execute sql
		auto rc = step(*stmthandle);
		if (rc != SQLITE_DONE && rc != SQLITE_ROW)
		{
			return std::make_pair(false, stmthandle);
		}
		else
		{
			return std::make_pair(true, stmthandle);
		}
	}

private:	
	database db;
};