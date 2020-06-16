#pragma once
#include <tuple>
#include "database.hpp"
#include "order.hpp"
namespace warehouse
{
	class order_manager
	{
	private:
		// Database instance handle
		database* db;
	public:
		// No default ctor
		// Param ctor
		order_manager(database* pdb) :
			db(pdb) {}
		// Copy ctor
		order_manager(const order_manager& oth) :
			db(oth.db) {}
		// Copy-assignments
		order_manager& operator =(const order_manager& rhs)
		{
			if (this != &rhs)
			{
				db = rhs.db;
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
		struct statement_generator
		{
			inline statement_handle insert_order_stmt()
			{
				static const char sqlstmt[] =
					"INSERT INTO order(id, status) VALUES($oid, $s)";

				return statement_handle{ sqlstmt, **db };
			}

			inline statement_handle insert_order_detail_stmt()
			{
				static const char sqlstmt[] =
					"INSERT INTO order_detail(order_id, item_id, quantity) VALUES($oid, $iid, $q)";

				return statement_handle{ sqlstmt, **db };
			}

			inline statement_handle insert_refund_order_stmt()
			{
				static const char sqlstmt[] =
					"INSERT INTO refund_order(order_id, item_id, refund_quantity) VALUES($oid, $iid, $rq)";

				return statement_handle{ sqlstmt, **db };
			}

			inline statement_handle query_order_stmt()
			{
				static const char sqlstmt[] =
					"SELECT * FROM order WHERE order_id = $oid";

				return statement_handle{ sqlstmt, **db };
			}

			inline statement_handle query_order_detail_stmt()
			{
				static const char sqlstmt[] =
					"SELECT * FROM order_detail WHERE order_id = $oid";

				return statement_handle{ sqlstmt, **db };
			}

			inline statement_handle query_refund_order_stmt()
			{
				static const char sqlstmt[] =
					"SELECT * FROM refund_order WHERE order_id = $oid";

				return statement_handle{ sqlstmt, **db };
			}
			inline statement_handle update_status_stmt()
			{
				static const char sqlstmt[] =
					"UPDATE order SET status = $s WHERE order_id = $oid";

				return statement_handle{ sqlstmt, **db };
			}
		};
		

		void extract_query(Order&);
		void extract_query(Refund_Order&);

		void bind_insert(const Order&);
		void bind_insert(const Refund_Order&);

		int step();
	};
}