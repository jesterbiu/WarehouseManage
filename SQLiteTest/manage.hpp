#pragma once
#include <iostream>
#include <tuple>
#include <memory>
#include "sqlite3.h"
#include "data.hpp"

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
public:
	
	manage() :
		db(":memory:")
	{
		// Create table
		if (db)
		{
			const char sqlstmt[] = 
				"CREATE TABLE data_table (id TEXT, location TEXT, stocks INTEGER)";
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
		
	bool add(const data& d)
	{
		// Validate input
		if (!db)
		{
			return false;
		}

		// Prepare stmthandle
		auto sqlstmt =
			"INSERT INTO data_table(id, location, stocks) VALUES($id_, $location_, $stocks_)";
		auto stmthandle = prep_stmt(sqlstmt);
		if (!stmthandle)
		{
			return false;
		}

		// Bind				
		bind_rdata(d, stmthandle);
		
		// Execute
		step(stmthandle);

		// End
		sqlite3_finalize(stmthandle);

		return true;
	}

	std::tuple<bool, data> find(const std::string& id)
	{
		// Construct a default data
		auto d = data{};

		// Validate input
		if (!db)
		{
			return std::make_tuple(false, d);
		}

		// Prepare stmthandle
		auto sqlstmt =
			"SELECT id, location, stocks FROM data_table WHERE id = $id_";
		auto stmthandle = prep_stmt(sqlstmt);
		if (!stmthandle)
		{
			return std::make_tuple(false, d);
		}

		// Bind				
		sqlite3_bind_text(
			stmthandle,
			1,
			id.c_str(),
			id.size(),	
			nullptr		// no deleter
			);

		
		// Execute
		auto rc = step(stmthandle);
		if (rc != SQLITE_DONE && rc != SQLITE_ROW)
		{
			return std::make_tuple(false, d);
		}

		// Extract results
		d.ID = (const char*)sqlite3_column_text(stmthandle, 0);
		d.loc = (const char*)sqlite3_column_text(stmthandle, 1);
		d.stocks = sqlite3_column_int(stmthandle, 2);
		
		// End
		sqlite3_finalize(stmthandle);

		return std::make_tuple(true, d);
	}

	sqlite3_stmt* prep_stmt(const char* sqlstmt)
	{
		sqlite3_stmt* stmthandle;
		auto rc = sqlite3_prepare_v2(
			*db,
			sqlstmt,
			-1,						// read to the first '\0'
			&stmthandle,
			nullptr					// pointer to the past-end compiled pos
			);
		if (rc != SQLITE_OK)
		{
			std::cout << "prep_stmt error: " << rc << "\n";
			return nullptr;
		}

		return stmthandle;
	}

	// Bind data to write to (QUERY)
	void bind_wdata(data& d, sqlite3_stmt* stmthandle)
	{
		static size_t buf_sz = 50;

		// Bind ID
		auto id = std::make_unique<char[]>(buf_sz);
		sqlite3_bind_text(
			stmthandle,
			1,
			id.get(),	
			buf_sz - 1,	// the offset of '\0'
			nullptr		// no deleter
			);

		// Bind loc
		auto loc = std::make_unique<char[]>(buf_sz);
		sqlite3_bind_text(
			stmthandle,
			2,
			loc.get(),
			buf_sz - 1,	// the offset of '\0'
			nullptr		// no deleter
			);

		// Bind stocks
		auto stocks = d.stocks;
		sqlite3_bind_int(stmthandle, 3, stocks);
	}

	// Bind data to read from (INSERT, UPDATE)
	void bind_rdata(const data& d, sqlite3_stmt* stmthandle)
	{
		// Bind ID
		const char* id = d.ID.c_str();
		sqlite3_bind_text(
			stmthandle,
			1,
			id,
			d.ID.size(),	// the offset of '\0'
			nullptr			// no deleter
			);

		// Bind loc
		const char* loc = d.loc.c_str();
		sqlite3_bind_text(
			stmthandle,
			2,
			loc,
			d.loc.size(),	// the offset of '\0'
			nullptr			// no deleter
			);

		// Bind stocks
		auto stocks = d.stocks;
		sqlite3_bind_int(stmthandle, 3, stocks);
	}
	
	// Fetch a row of data and print error if there is any
	int step(sqlite3_stmt* stmthandle)
	{
		auto rc = sqlite3_step(stmthandle);
		if (rc != SQLITE_DONE)
		{
			std::cout << "add: ";
			switch (rc)
			{
			case SQLITE_BUSY:
				std::cout << "SQLITE_BUSY\n";
				break;
			case SQLITE_ERROR:
				std::cout << "SQLITE_ERROR\n";
				break;
			case SQLITE_MISUSE:
				std::cout << "SQLITE_MISUSE\n";
				break;
			default:
				std::cout << "\n";
				break;
			}
		}
		return rc;
	}
	

private:	
	database db;

};