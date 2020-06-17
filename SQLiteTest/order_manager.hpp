#pragma once
#include <tuple>
#include "manager.hpp"
#include "order.hpp"
namespace warehouse
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
		
		bool add_order(const Order& o);
		bool refund_order(const Refund_Order& ro);
		bool exist(const std::string& order_id);

		std::pair<bool, Order_Status_Type> check_status(const std::string& order_id);
		bool update_status(const std::string& order_id, const Order_Status_Type nstatus);
		
		std::pair<bool, Order> get_order(const std::string& order_id);
		std::pair<bool, Refund_Order> ger_refund_order(const std::string& order_id);
	private:
		struct order_statement_generator : public manager::statement_generator
		{
			inline static statement_handle insert_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"INSERT INTO order(id, status) VALUES($oid, $s)";

				return statement_handle{ sqlstmt, db };
			}

			inline static statement_handle insert_order_detail_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"INSERT INTO order_detail(order_id, item_id, quantity) VALUES($oid, $iid, $q)";

				return statement_handle{ sqlstmt, db };
			}

			inline static statement_handle insert_refund_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"INSERT INTO refund_order(order_id, item_id, refund_quantity) VALUES($oid, $iid, $rq)";

				return statement_handle{ sqlstmt, db };
			}

			inline static statement_handle order_exist_stmt(sqlite3* db)
			{
				static const char sqlstmt[]
					= "SELECT COUNT(1) FROM order WHERE order_id = $oid";
				return statement_handle{ sqlstmt, db };
			}

			inline static statement_handle query_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT * FROM order WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}

			inline static statement_handle query_order_detail_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT item_id, quantity FROM order_detail WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}

			inline static statement_handle query_refund_order_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"SELECT * FROM refund_order WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}
			
			inline static statement_handle update_status_stmt(sqlite3* db)
			{
				static const char sqlstmt[] =
					"UPDATE order SET status = $s WHERE order_id = $oid";

				return statement_handle{ sqlstmt, db };
			}
		};
		
		// add_order() helper functions
		bool insert_order(const Order& order);
		bool insert_detail(const Order& order);
		void bind_insert_order(const Order&, sqlite3_stmt* order_stmth);
		void bind_insert_detail(const std::string& order_id, const good& g, sqlite3_stmt* detail_stmth);

		void extract_order_query(Order&, sqlite3_stmt*);
		void extract_detail_query(Order&, sqlite3_stmt*);
		void extract_refund_query(Refund_Order&, sqlite3_stmt*);
				
		void bind_insert_refund(const Refund_Order&);
	};
}