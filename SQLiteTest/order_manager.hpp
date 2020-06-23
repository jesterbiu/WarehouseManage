#pragma once
#include <tuple>
#include "manager.hpp"
#include "order.hpp"
namespace WarehouseManage
{

	class order_manager : public manager
	{	
	public:
		// No default ctor
		order_manager() = delete;
		// Param ctor
		order_manager(database* pdb) :
			manager(pdb) {}
		// Copy ctor
		order_manager(const order_manager& oth) :
			manager(oth) {}
		// Copy-assignments
		order_manager& operator =(const order_manager& rhs)
		{
			if (this != &rhs)
			{
				manager::operator=(rhs);
			}
			return *this;
		}
		
		// Add a new order to database; return true on success
		// REMEMBER TO UPADTE ITEMS!
		bool add_order(const Order& o);		
		
		// Add a new refund order to database; return true on success	
		// REMEMBER TO UPADTE ITEMS!
		bool add_refund_order(const Refund_Order& ro);		
		
		// Check if an order exists; return true if exists
		bool exist(const std::string& order_id);
		
		// Check and return an order's status given order id
		std::pair<bool, Order_Status_Type> check_status(const std::string& order_id);		
		
		// Update an order's status given order id
		bool update_status(const std::string& order_id, const Order_Status_Type nstatus);
		
		// Return the total count of orders
		int order_count();
		
		// Try to store all but max_count of orders in the container 
		// which the iter belongs to
		template<typename InputIt>
		void get_all_order_ids(InputIt iter, size_t max_count)
		{
			// Validate input
			if (max_count < 1)
			{
				return;
			}

			// Prepare SQL
			auto stmth = order_statement_generator::select_all_id_stmt(get_database());
			database::verify_stmt_handle(stmth);

			// Execute
			auto rc = database::step(*stmth);
			database::verify_steping(rc);

			// Extract	
			auto i = 0;
			while (database::step_has_result(rc) && i != max_count)
			{
				// Extract current row				
				*iter = database::extract_text(*stmth, 0);

				// Try to fetch the next row
				auto rc = database::step(*stmth);
				database::verify_steping(rc);

				// Increment
				iter++;
				i++;
			}
		}
		
		// Get order from database
		std::pair<bool, Order> get_order(const std::string& order_id);
		
		// Get refund order from database
		std::pair<bool, Refund_Order> get_refund_order(const std::string& order_id);

	private:
		// Return a statement_handler of a certain SQL statement
		struct order_statement_generator : public manager::statement_generator
		{
		// Schemas:
		// orders			(order_id TEXT, status	INTEGER)
		// order_details	(order_id TEXT, item_id TEXT,	quantity		INTEGER)
		// refund_orders	(order_id TEXT, item_id TEXT,	refund_quantity INTEGER)";
			inline static statement_handle insert_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"INSERT INTO orders(order_id, status) VALUES($oid, $s)";

				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle insert_order_detail_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"INSERT INTO order_details(order_id, item_id, quantity) VALUES($oid, $iid, $q)";

				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle insert_refund_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"INSERT INTO refund_orders(order_id, item_id, refund_quantity) VALUES($oid, $iid, $rq)";

				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle order_exist_stmt(sqlite3* db)
			{
				static const char sqlstmt[]
					= "SELECT COUNT(1) FROM orders WHERE order_id = $oid";
				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle query_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT * FROM orders WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle query_order_status_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT status FROM orders WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle query_order_detail_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT item_id, quantity FROM order_details WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}
			inline static statement_handle query_refund_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT item_id, refund_quantity FROM refund_orders WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}			
			inline static statement_handle update_status_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"UPDATE orders SET status = $s WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}		
			inline static statement_handle count_order_stmt(sqlite3* db)
			{
				static auto sqlstmt
					= "SELECT COUNT(*) FROM orders";
				return statement_handle{ sqlstmt, db };
			}			
			inline static statement_handle select_all_id_stmt(sqlite3* db)
			{
				static auto sqlstmt
					= "SELECT order_id FROM orders";
				return statement_handle{ sqlstmt, db };
			}
		};
		
		// add_order() helper functions
		bool insert_order_part(const Order& order);
		bool insert_detail_part(const Order& order);
		void bind_insert_order_part(const Order&, sqlite3_stmt* order_stmth);
		void bind_insert_detail_part(const std::string& order_id, const good& g, sqlite3_stmt* detail_stmth);
	

		// get_order() helper functions
		void get_order_part(Order&);
		void get_detail_part(Order&);
		void extract_order_query(Order&, sqlite3_stmt*);
		void extract_detail_query(Order&, sqlite3_stmt*);

		void extract_refund_query(Refund_Order&, sqlite3_stmt*);
				
		// add_refund_order() helper function
		void bind_insert_refund(const std::string & order_id, const good&, sqlite3_stmt*);
	};
}